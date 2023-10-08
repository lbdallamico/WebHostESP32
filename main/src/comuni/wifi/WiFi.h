#ifndef WiFI_H_
#define WiFI_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <iostream>
#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>

// #include <nvs>
//  * As configurações de WiFI estão nesta class
#include "WiFi_Settings.h"

#define TAG_HW_WIFI "[WiFi HW]"

class WiFi
{
public:
    typedef enum
    {
        kNone,
        kAP,
        kSTA,
        kAP_STA,
    } wifi_tipo_rede_t;

public:
    ~WiFi();
    bool Begin(wifi_tipo_rede_t config);
    void Destruct();
    bool Start();
    bool GetWifiStatus();
    wifi_tipo_rede_t GetWifiMode();
    static WiFi *GetObjs();
    uint8_t GetAPdeviceconnected();
    bool SetSsidAndPassword(std::string ssid, std::string password, wifi_auth_mode_t autenticao = WIFI_AUTH_WPA_WPA2_PSK);

protected:
    static SemaphoreHandle_t _semaphoro_wifi_mode;
    static uint8_t _sta_devices_connected;

private:
    WiFi();
    /* Cria a rede */
    bool MakeAPrede();
    bool MakeSTArede();
    bool InitNetif();
    /* Inicializa a rede */
    bool StartAPrede();
    bool StartSTArede();
    /* Task que rodam paralela */
    static void ScanRSSI(void *args);
    static void ManagerAP_STA(void *args);
    /* feedback by event */
    static void event_handler_wifi(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    /* Variables */
    static bool _is_connected;
    static WiFi *_ptr_wifi;
    wifi_tipo_rede_t _type_connection;
    wifi_config_t _ap_config;
    wifi_config_t _sta_config;
    std::string _ssid;
    std::string _password;
    static TaskHandle_t _control_ScanRSSI;
    static TaskHandle_t _control_ManagerAP_STA;
};

#endif