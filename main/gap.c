/*
 * gap.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "stack/gap_api.h"
#include "stack/bt_types.h"
#include "osi/allocator.h"

#include "include/gap.h"
#include "include/parse.h"

static void gap_event_handle(uint16_t handle, uint16_t event) {
  switch (event) {
    case GAP_EVT_CONN_DATA_AVAIL: {
      BT_HDR *p_buf;

      GAP_ConnBTRead(handle, &p_buf);

      if (p_buf->len > 0) {
          //ESP_LOGE("ESP32BT", "%s GAP Data Available", __func__);
          parse_packet(p_buf->len, p_buf->offset, p_buf->data);
      }

      osi_free(p_buf);

      break;
    }
  }
}

bool gap_init_hardware() {
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

  if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
      ESP_LOGE("ESP32BT", "Bluetooth controller initialize failed");
      return false;
  }

  if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
      ESP_LOGE("ESP32BT", "Bluetooth controller enable failed");
      return false;
  }

  if (esp_bluedroid_init() != ESP_OK) {
    ESP_LOGE("ESP32BT", "Bluedroid initialize failed");
    return false;
  }

  if (esp_bluedroid_enable() != ESP_OK) {
    ESP_LOGE("ESP32BT", "Bluedroid enable failed");
    return false;
  }

  return true;
}

uint16_t gap_init_service(char *name, uint16_t psm, uint8_t security_id) {
    uint16_t handle = GAP_ConnOpen (name, security_id, /*is_server=*/false, /*p_rem_bda=*/gamepad_btaddr,
                     psm, &cfg_info, &ertm_info, /*security=*/0, /*chan_mode_mask=*/1,
                     gap_event_handle);

    if (handle == GAP_INVALID_HANDLE){
        ESP_LOGE("ESP32BT", "%s Registering GAP service %s failed", __func__, name);
    }else{
        ESP_LOGI("ESP32BT", "[%s] GAP Service %s Initialized: %d", __func__, name, handle);
    }

    return handle;
}
