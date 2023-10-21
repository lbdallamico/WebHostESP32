#include "WiFi.h"

#include <esp_system.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <esp_wifi_types.h>
#include <esp_intr_alloc.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_wifi_default.h>
#include <esp_wifi_default.h>
// #include <nvs_flash.h>

#include <lwip/err.h>
#include <lwip/sys.h>

WiFi *WiFi::_ptr_wifi = nullptr;
SemaphoreHandle_t WiFi::_semaphoro_wifi_mode = nullptr;
uint8_t WiFi::_sta_devices_connected = 0;
TaskHandle_t WiFi::_control_ScanRSSI = nullptr;
TaskHandle_t WiFi::_control_ManagerAP_STA = nullptr;
bool WiFi::_is_connected = 0;

/**
 * @brief Destrutor
 */
WiFi::~WiFi() {}

/**
 * @brief Cria a estrutura de configuração de wifi
 * @param config tipo de rede para construir
 * @return 1 - Sucesso, 0 - Falhou
 */
bool WiFi::Begin(wifi_tipo_rede_t config)
{
    ESP_LOGD(TAG_HW_WIFI, "Wifi Begin is called");

    _is_connected = 0;
    _type_connection = config;
    /**
     * TODO: Aqui tem problema de estrutura
     *
     * Pq eu deveria estar garantindo que nunca vou apagar a rede
     * Pq n faz sentido ter o gerenciamento
     *
     *
     */
    _ssid.clear();
    _password.clear();
    _semaphoro_wifi_mode = xSemaphoreCreateMutex();
    if (_semaphoro_wifi_mode == nullptr)
    {
        ESP_LOGE(TAG_HW_WIFI, "Error to create semaphoro wifi");
        return false;
    }

    bool result = false;
    result = InitNetif();

    switch (config)
    {
    case kAP:
        result = MakeAPrede();
        break;

    case kSTA:
        result = MakeSTArede();
        break;

    case kAP_STA:
        result = MakeAPrede();
        result = MakeSTArede();
        break;

    case kNone:
        ESP_LOGE(TAG_HW_WIFI, "Tipo de wifi não especificado");
        result = false;
        break;

    default:
        ESP_LOGE(TAG_HW_WIFI, "Wifi config isn't support");
        _type_connection = kNone;
        result = false;
        break;
    }
    return result;
}

/**
 * @brief Destroi a rede criada e limpa as estrutura de wifi
 * @return void
 */
void WiFi::Destruct()
{
    // eu preciso destruir as task que criei
    // limpar os events para n duplicar
    esp_wifi_stop();
    esp_wifi_deinit();

    // remove os eventos de feedback
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler_wifi));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_wifi));

    // delete task que foram criadas
    vTaskDelete(_control_ManagerAP_STA);

    // verificar se as tasks
    _is_connected = 0;
    _type_connection = kNone;
    _ssid.clear();
    _password.clear();
}

/**
 * @brief Inicia a rede configurada em WiFi::Begin
 * @return bool
 */
bool WiFi::Start()
{
    ESP_LOGD(TAG_HW_WIFI, "Wifi Start Start");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler_wifi, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_wifi, NULL));

    bool result = false;
    BaseType_t xReturned = pdFAIL;

    switch (_type_connection)
    {
    case (kAP):
        result = StartAPrede();
        break;

    case (kSTA):
        result = StartSTArede();
        break;

    case kAP_STA:
        /**
         * Começa em default como AP para se configurar
         */
        result = StartAPrede();
        xReturned = xTaskCreate(
            ManagerAP_STA,
            "ManagerAP_STA",
            TASK_STACK_REF_WIFI,
            NULL,
            FREERTOS_PRIORITY_TASK,
            &_control_ManagerAP_STA);

        if (xReturned != pdPASS)
        {
            ESP_LOGE(TAG_HW_WIFI, "ScanRSSI não foi criado");
        }
        break;

    case kNone:
        ESP_LOGW(TAG_HW_WIFI, "Wifi Start nao configurado");
        return false;

    default:
        ESP_LOGW(TAG_HW_WIFI, "Wifi Start nao especificado");
        return false;
        break;
    }
    return result;
}

/**
 * @brief Retorna o estado de conexao quando está em modo STA
 * @result 1 - Conectado, 0 - Desconectado
 */
bool WiFi::GetWifiStatus()
{
    if (_type_connection == kSTA)
    {
        return _is_connected;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Retorna o objeto da classe WiFI
 * @return Ponteiro WiFi
 */
WiFi *WiFi::GetObjs()
{
    if (_ptr_wifi == nullptr)
    {
        // limpa a estrutura
        _is_connected = 0;
        _ptr_wifi = new WiFi();
        if (_ptr_wifi == nullptr)
        {
            ESP_LOGE(TAG_HW_WIFI, "Erro to create WiFi object");
            abort();
        }
        else
        {
            ESP_LOGD(TAG_HW_WIFI, "WiFI was created as sucess");
        }
        return _ptr_wifi;
    }
    else
    {
        return _ptr_wifi;
    }
}

/**
 * @brief Retorna a quantidade de objetos conectados quando está em modo AP
 */
uint8_t WiFi::GetAPdeviceconnected()
{
    return _sta_devices_connected;
}

/**
 * @brief Construtor Privado
 */
WiFi::WiFi() {}

/**
 * @brief Cria uma rede em modo AP
 * @return bool
 */
bool WiFi::MakeAPrede()
{
    ESP_LOGD(TAG_HW_WIFI, "Wifi MakeAPrede is called");
    esp_netif_create_default_wifi_ap();

    wifi_config_t _default_ap_config = {
        .ap = {
            .ssid_len = DEFAULT_SOFT_AP_SSID_LENGHT,
            .channel = DEFAULT_SOFT_AP_CHANNEL,
            .authmode = DEFAULT_SOFT_AP_AUTH_MODE,
            .ssid_hidden = DEFAULT_SOFT_AP_SSID_HIDDEN,
            .max_connection = DEFAULT_SOFT_AP_MAX_CONNECTIONS,
            .beacon_interval = DEFAULT_SOFT_AP_BEACON_INTERVAL,
            // doesn't matter
            .pairwise_cipher = WIFI_CIPHER_TYPE_NONE,
            .ftm_responder = false}};

    strcpy((char *)_default_ap_config.ap.ssid, DEFAULT_SOFT_AP_SSID);
    strcpy((char *)_default_ap_config.ap.password, DEFAULT_SOFT_AP_PASSWORD);

    _ap_config = _default_ap_config;
    return true;
}

/**
 * @brief Cria uma rede em modo STA
 * @return bool
 */
bool WiFi::MakeSTArede()
{
    ESP_LOGD(TAG_HW_WIFI, "Wifi MakeSTArede is called");

    esp_netif_create_default_wifi_sta();

    memset(&_sta_config, 0, sizeof(wifi_sta_config_t));

    return true;
}

/**
 * @brief Verifica os dispositivos conectados a rede quando está em modo AP
 * @note TASK do FreeRTOS
 */
void WiFi::ScanRSSI(void *args)
{
    while (1)
    {
        if (xSemaphoreTake(WiFi::_semaphoro_wifi_mode, MAX_TIME_WAIT_ON_SEMAPHORO / portTICK_PERIOD_MS) != pdTRUE)
        {
            ESP_LOGW(TAG_HW_WIFI, "Nao foi possivel pegar o mutex!");
            return;
        }

        ESP_LOGD(TAG_HW_WIFI, "Verificando o RSSI");

        wifi_sta_list_t sta;
        esp_err_t err = esp_wifi_ap_get_sta_list(&sta);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG_HW_WIFI, "esp_wifi_ap_get_sta_list(&sta) error = %s", esp_err_to_name(err));
        }
        else
        {
            if (sta.num >= 0)
            {
                WiFi::_sta_devices_connected = sta.num;

                ESP_LOGD(TAG_HW_WIFI, "Numero de conexoes: %d", sta.num);
                ESP_LOGD(TAG_HW_WIFI, "RSSI[0] do celular: %d", sta.sta[0].rssi);

                if (sta.num > 0 && sta.sta[0].rssi < MAIN_RSSI_CHECK_TIMER_MIN_RSSI)
                {
                    ESP_LOGW(TAG_HW_WIFI, "O sinal esta muito fraco");
                    esp_wifi_stop();
                    vTaskDelay(MAIN_RSSI_CHECK_TIMER_RESTART_DELAY_MS / portTICK_PERIOD_MS);
                    esp_wifi_start();
                }
            }
            else
            {
                WiFi::_sta_devices_connected = 0;
                ESP_LOGD(TAG_HW_WIFI, "Numero de conexoes: %d", sta.num);
            }
        }
        xSemaphoreGive(WiFi::_semaphoro_wifi_mode);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Troca entre AP e STA para enviar informações ao servidor
 * @note TASK do FreeRTOS
 * TODO AQUI
 */
void WiFi::ManagerAP_STA(void *args)
{
    if (xSemaphoreTake(WiFi::_semaphoro_wifi_mode, MAX_TIME_WAIT_ON_SEMAPHORO / portTICK_PERIOD_MS) != pdTRUE)
    {
        ESP_LOGW(TAG_HW_WIFI, "Nao foi possivel pegar o mutex!");
        return;
    }

    // if (_type_connection)

    xSemaphoreGive(WiFi::_semaphoro_wifi_mode);
}

/**
 * @brief Inicia a abstrção da netfit do esp
 * @return bool
 */
bool WiFi::InitNetif()
{
    // Cria uum grupo para wifi
    // EventGroupHandle_t s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    return true;
}

/**
 * @brief Retorna o tipo de wifi definido na classe WiFI
 */
WiFi::wifi_tipo_rede_t WiFi::GetWifiMode()
{
    return WiFi::_type_connection;
}

/**
 * @brief Defini uma ssid e um password para rede
 * @note Vale tanto para AP quanto para STA
 *
 * @param ssid nome da rede
 * @param password senha da rede
 * @param autenticao tipo de autenticação de rede
 */
bool WiFi::SetSsidAndPassword(std::string ssid, std::string password, wifi_auth_mode_t autenticao)
{
    // Salva o set
    wifi_tipo_rede_t local_type = _type_connection;
    bool was_active = false;

    if (xSemaphoreTake(WiFi::_semaphoro_wifi_mode, MAX_TIME_WAIT_ON_SEMAPHORO / portTICK_PERIOD_MS) != pdTRUE)
    {
        ESP_LOGW(TAG_HW_WIFI, "Nao foi possivel pegar o mutex!");
        return false;
    }

    if (GetWifiStatus() > 0 || GetAPdeviceconnected() > 0)
    {
        was_active = true;
        Destruct();
        Begin(local_type);
    }

    if (_type_connection == kSTA || _type_connection == kAP_STA)
    {
        // Limpar a estrutra
        memset((char *)WiFi::_sta_config.sta.ssid    , 0, sizeof(WiFi::_sta_config.sta.ssid ));
        memset((char *)WiFi::_sta_config.sta.password, 0, sizeof(WiFi::_sta_config.sta.password));
        // Grava novo valor
        strcpy((char *)WiFi::_sta_config.sta.ssid    , ssid.c_str());
        strcpy((char *)WiFi::_sta_config.sta.password, password.c_str());
        _sta_config.sta.threshold.authmode = autenticao;
    }
    else if (_type_connection == kAP || _type_connection == kAP_STA)
    {
        // Limpar a estrutra
        memset((char *)WiFi::_ap_config.sta.ssid    , 0, sizeof(WiFi::_ap_config.sta.ssid ));
        memset((char *)WiFi::_ap_config.sta.password, 0, sizeof(WiFi::_ap_config.sta.password));
        // Grava novo valor
        strcpy((char *)WiFi::_ap_config.sta.ssid    , ssid.c_str());
        strcpy((char *)WiFi::_ap_config.sta.password, password.c_str());
        _sta_config.sta.threshold.authmode = autenticao;
    }
    else
    { /* Do Nothing */
    }

    // Verifica se a rede precisou ser destruida
    if (was_active)
    {
        Start();
    }

    xSemaphoreGive(WiFi::_semaphoro_wifi_mode);
    return true;
}

/**
 * @brief Callback do modulo de wifi
 *
 * @note Aqui é tratado toda a alteração de estado do wifi (conexao, desconexao)
 * A implementação alimenta outras do sistema
 */
void WiFi::event_handler_wifi(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    BaseType_t xReturned = pdFAIL;
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        /**
         * * MODO STA
         */
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG_HW_WIFI, "WIFI_EVENT_STA_START");
            WiFi::_is_connected = false;
            break;
        case WIFI_EVENT_STA_STOP:
            WiFi::_is_connected = false;
            ESP_LOGI(TAG_HW_WIFI, "WIFI_EVENT_STA_STOP");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            WiFi::_is_connected = false;
            ESP_LOGI(TAG_HW_WIFI, "WIFI_EVENT_STA_CONNECTED");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG_HW_WIFI, "WIFI_EVENT_STA_DISCONNECTED");
            WiFi::_is_connected = false;
            esp_wifi_connect();
            break;
        /**
         * * MODO AP
         */
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG_HW_WIFI, "WIFI_EVENT_AP_START");
            xReturned = xTaskCreate(
                ScanRSSI,
                "ScanRSSI",
                TASK_STACK_REF_WIFI,
                NULL,
                FREERTOS_PRIORITY_TASK,
                &_control_ScanRSSI);

            if (xReturned != pdPASS)
            {
                ESP_LOGE(TAG_HW_WIFI, "ScanRSSI não foi criado");
            }
            break;

        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG_HW_WIFI, "WIFI_EVENT_AP_STOP");
            vTaskDelete(_control_ScanRSSI);
            break;
        /**
         * * WRONG CONFIG
         */
        default:
            // ESP_LOGI(TAG_HW_WIFI, "Feedback WIFI_EVENT não especificada = %d", event_id);
            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG_HW_WIFI, "IP_EVENT_STA_GOT_IP");
            WiFi::_is_connected = true;
            break;

        case IP_EVENT_STA_LOST_IP:
            ESP_LOGI(TAG_HW_WIFI, "IP_EVENT_STA_LOST_IP");
            WiFi::_is_connected = false;
            break;

        default:
            // ESP_LOGI(TAG_HW_WIFI, "Feedback IP_EVENT não especificada = %d", event_id);
            break;
        }
    }
}

/**
 * @brief Inicia a rede em modo AP
 * @return bool
 */
bool WiFi::StartAPrede()
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &_ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    return true;
}

/**
 * @brief Inicializa a rede em modo STA
 * @return result
 */
bool WiFi::StartSTArede()
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
    return true;
}
