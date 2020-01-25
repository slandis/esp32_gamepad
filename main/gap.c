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

bool targetFound = false;

uint16_t gap_handle_hidc = GAP_INVALID_HANDLE;
uint16_t gap_handle_hidi = GAP_INVALID_HANDLE;

static tL2CAP_ERTM_INFO ertm_info = {0};
static tL2CAP_CFG_INFO cfg_info = {0};

static uint16_t gap_init_service(char *, uint16_t psm, uint8_t);
static void gap_event_handle(uint16_t handle, uint16_t event);

static bool is_target_addr(uint8_t *addr) {
  for (int i = 0; i < BD_ADDR_LEN; i++) {
    if (addr[i] != gamepad_btaddr[i]) {
      return false;
    }
  }

  return true;
}

static void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT: {
      if (is_target_addr(param->disc_res.bda) && !targetFound) {
        targetFound = true;
        ESP_LOGI("ESP32BT", "Attempting to connect");
        gap_handle_hidc = gap_init_service( "HIDC", BT_PSM_HIDC, BTM_SEC_SERVICE_FIRST_EMPTY   );
        gap_handle_hidi = gap_init_service( "HIDI", BT_PSM_HIDI, BTM_SEC_SERVICE_FIRST_EMPTY+1 );
      }

      break;
    }

    default:
      break;
  }
}

static void gap_event_handle(uint16_t handle, uint16_t event) {
  switch (event) {
    case GAP_EVT_CONN_OPENED: {
      ESP_LOGI("ESP32BT", "GAP Connection Opened: %d", handle);
      esp_bt_gap_cancel_discovery();
      break;
    }

    case GAP_EVT_CONN_CLOSED: {
      targetFound = false;
      esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 0x30, 0x0);
      break;
    }

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

  esp_bt_gap_register_callback(gap_callback);

  if (esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 0x30, 0x0) != ESP_OK) {
    ESP_LOGE("ESP32BT", "Unable to start discovery service");
    return false;
  }

  return true;
}

static uint16_t gap_init_service(char *name, uint16_t psm, uint8_t security_id) {
    uint16_t handle = GAP_ConnOpen(name, security_id, /*is_server=*/false, /*p_rem_bda=*/gamepad_btaddr,
                     psm, &cfg_info, &ertm_info, /*security=*/0, /*chan_mode_mask=*/0,
                     gap_event_handle);

    if (handle == GAP_INVALID_HANDLE){
        ESP_LOGE("ESP32BT", "%s Registering GAP service %s failed", __func__, name);
    }else{
        ESP_LOGI("ESP32BT", "[%s] GAP Service %s Initialized: %d", __func__, name, handle);
    }

    return handle;
}
