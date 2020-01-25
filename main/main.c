#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "stack/bt_types.h"
#include "stack/gap_api.h"

#include "include/gap.h"

uint16_t gap_handle_hidc = GAP_INVALID_HANDLE;
uint16_t gap_handle_hidi = GAP_INVALID_HANDLE;

uint8_t gamepad_btaddr[BD_ADDR_LEN] = {0x28, 0x9A, 0x4B, 0x0A, 0x1D, 0x9A};

void mainTask(void *pvParameters) {
  while (GAP_INVALID_HANDLE == gap_handle_hidc || GAP_INVALID_HANDLE == gap_handle_hidi) {
    ESP_LOGI("ESP32BT", "Attempting to connect to bluetooth device");
    gap_handle_hidc = gap_init_service( "HIDC", BT_PSM_HIDC, BTM_SEC_SERVICE_FIRST_EMPTY   );
    gap_handle_hidi = gap_init_service( "HIDI", BT_PSM_HIDI, BTM_SEC_SERVICE_FIRST_EMPTY+1 );

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  nvs_flash_init();

  if (!gap_init_hardware()) {
    ESP_LOGE("ESP32BT", "Unable to initiate bluetooth hardware.");
    return;
  }

  xTaskCreatePinnedToCore(&mainTask, "mainTask", 2048, NULL, 5, NULL, 0);
}
