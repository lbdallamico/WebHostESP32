#ifndef WiFi_Settings_H_
#define WiFi_Settings_H_

// SoftAP
#define DEFAULT_SOFT_AP_SSID                        "Sem Rede ESP"
#define DEFAULT_SOFT_AP_PASSWORD                    "123456789"
#define DEFAULT_SOFT_AP_SSID_LENGHT                 sizeof(DEFAULT_SOFT_AP_SSID)
#define DEFAULT_SOFT_AP_CHANNEL                     3
#define DEFAULT_SOFT_AP_AUTH_MODE                   wifi_auth_mode_t::WIFI_AUTH_WPA_WPA2_PSK
#define DEFAULT_SOFT_AP_SSID_HIDDEN                 0
#define DEFAULT_SOFT_AP_MAX_CONNECTIONS             1
#define DEFAULT_SOFT_AP_BEACON_INTERVAL             100

// ! REDEFINICAO
#ifndef CONFIG_ESP32_WIFI_STATIC_RX_BUFFER_NUM
    #define CONFIG_ESP32_WIFI_STATIC_RX_BUFFER_NUM          10
#endif

/**
 * Especificacao de produto
*/
#define MAIN_RSSI_CHECK_TIMER_INTERVAL                      3000
#define MAIN_RSSI_CHECK_TIMER_RESTART_DELAY_MS              5000
#define MAIN_RSSI_CHECK_TIMER_MIN_RSSI                      -75

#define MAX_TIME_WAIT_ON_SEMAPHORO                          1000 

#define TASK_STACK_REF_WIFI                                 8*1024
#define FREERTOS_PRIORITY_TASK                              6

#endif