#include <stdint.h>
#include <string>
// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
// esp
#include "nvs_flash.h"
#include "esp_log.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
// Intern libs

#include "WiFi.h"

/*
 * =================================
 *          Variables Local
 * =================================
 */

/*
 * =================================
 *              START
 * =================================
 */
void setup(void)
{
  // Start Wifi on AP mode
  WiFi *m_wifi = WiFi::GetObjs();
  m_wifi->WiFi::SetSsidAndPassword("Redezera", "12345678");
  m_wifi->Begin(kAP);

  // Start webserver
  WebServer * m_web = WebServer::GetObjs();
  m_web->Begin();

  while(1)
  {
    m_web->IsActive();
    vTaskDelay(500);
  }
}


/*
 * =================================
 *         MAIN FreeRTOS
 * =================================
 */
extern "C"
{
  void app_main(void)
  {
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
      ESP_LOGI(TAG_MAIN, "Problema na NVS %s", esp_err_to_name(error));
      ESP_ERROR_CHECK(nvs_flash_erase());
      error = nvs_flash_init();
    }

    ESP_LOGI(TAG_MAIN, "Inicializou a NVS %s", esp_err_to_name(error));
    ESP_ERROR_CHECK(error);

    // Init
    setup();
    vTaskDelete(NULL);
  }
}
