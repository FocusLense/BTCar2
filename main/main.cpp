#include "LEDCChannel.hpp"
#include "LEDCTimer.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "MAIN";

//service and characteristic uuid's
//6E 40 00 01 - B5 A3 - F3 93 - E0 A9- E5 0E 24 DC CA 9E
static const ble_uuid128_t svc_uuid =
    BLE_UUID128_INIT(0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,
        0x93,0xf3,0xa3,0xb5,0x01,0x00,0x40,0x6e);
//6E 40 00 02 - B5 A3 - F3 93 - E0 A9 - E5 0E 24 DC CA 9E
static const ble_uuid128_t ctrl_chr_uuid =
    BLE_UUID128_INIT(0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,
        0x93,0xf3,0xa3,0xb5,0x02,0x00,0x40,0x6e);

//motors before cb
static LEDCTimer motorTimer(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, LEDC_TIMER_8_BIT, 20000);
static LEDCChannel motorIn1(motorTimer, LEDC_CHANNEL_0, 3);
static LEDCChannel motorIn2(motorTimer, LEDC_CHANNEL_1, 4);

static LEDCTimer servoTimer(LEDC_LOW_SPEED_MODE, LEDC_TIMER_1, LEDC_TIMER_14_BIT, 50);
static LEDCChannel servo(servoTimer, LEDC_CHANNEL_2, 21);

//access callback
static int control_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg){
    //check if access is valid, otherwise do nothing
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
        return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
    }
    const uint16_t received_len = OS_MBUF_PKTLEN(ctxt->om);
    if (received_len != 2) {
        ESP_LOGW(TAG, "Expected 2 bytes");
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    uint8_t payload[2];
    uint16_t copied_len = 0;

    if (ble_hs_mbuf_to_flat(ctxt->om, payload, sizeof(payload), &copied_len) != 0 ||
        copied_len != received_len) {
        ESP_LOGE(TAG, "Could not read write payload");
        return BLE_ATT_ERR_UNLIKELY;
    }

    const int steering = static_cast<int8_t>(payload[0]);
    const int throttle = static_cast<int8_t>(payload[1]);

    if (steering < -100 || steering > 100 ||
        throttle < -100 || throttle > 100) {
        //ESP_LOGW(TAG, "Control values out of range: %d, %d", steering, throttle);
        return BLE_ATT_ERR_VALUE_NOT_ALLOWED;
    }

    ESP_LOGI(TAG, "BLE control received: steering=%d throttle=%d",
         steering, throttle);
    if(throttle >= 0){
        motorIn1.setDutyPercent(throttle);
        motorIn2.setDutyPercent(0);
    }
    else{
        //(-(-throttleValue)) to make it positive, because minus in the value is just for direction query
        motorIn2.setDutyPercent(-throttle);
        motorIn1.setDutyPercent(0);
    }

    const uint32_t servo_pulse_us = 1500 + steering * 5;
    const esp_err_t servo_err = servo.setPulseWidthUs(servo_pulse_us);
    if (servo_err != ESP_OK) {
        ESP_LOGE(TAG, "Could not update servo PWM: %s", esp_err_to_name(servo_err));
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

//gatt table
static const struct ble_gatt_chr_def gatt_chrs[] = {
    {
            .uuid = &ctrl_chr_uuid.u,
            .access_cb = control_access_cb,
            .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
    },
    {}
};


static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &svc_uuid.u,
        .characteristics = gatt_chrs,
    },
    {}
};
void gatt_svc_init(void){
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);
}

//gap event handler (connection handling)

static void start_adv();
static int gap_event_handler(struct ble_gap_event *event, void *args){
    switch(event->type){
        case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            ESP_LOGI(TAG, "BLE CONNECTED");
            struct ble_gap_upd_params params = {};
            params.itvl_min = 6;
            params.itvl_max = 12;
            params.latency = 0;
            params.supervision_timeout = 400;
            params.min_ce_len = 0;
            params.max_ce_len = 0;
            const int rc = ble_gap_update_params(event->connect.conn_handle, &params);
            if (rc != 0) {
                ESP_LOGW(TAG, "Could not request shorter BLE interval: %d", rc);
            }
        }
        break;
        case BLE_GAP_EVENT_CONN_UPDATE:
        if (event->conn_update.status == 0) {
            ESP_LOGI(TAG, "BLE connection interval updated");
        } else {
            ESP_LOGW(TAG, "BLE connection interval update failed: %d",
                     event->conn_update.status);
        }
        break;
        case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG,"BLE DISCONNECTED; RESTARTING ADV");
        start_adv();
    }
    return 0;
}
static void start_adv(){
    struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN|BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen((char *)fields.name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params = {0};
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    uint8_t own_addr_type;
    ble_hs_id_infer_auto(0,&own_addr_type);
    uint8_t addr[6];
    int rc = ble_hs_id_copy_addr(own_addr_type, addr, nullptr);
    if (rc != 0) {
        ESP_LOGE(TAG, "Could not get BLE address: %d", rc);
    } else {
        ESP_LOGI(TAG, "BLE address: %02X:%02X:%02X:%02X:%02X:%02X",
                addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    }
    ble_gap_adv_start(own_addr_type,NULL,BLE_HS_FOREVER,&adv_params,gap_event_handler,NULL);
}

static void on_sync(){
    start_adv();
}
static void host_task(void *param){
    nimble_port_run();
}
    
extern "C" void app_main(){
    //init motors
    ESP_ERROR_CHECK(motorTimer.init());
    ESP_ERROR_CHECK(motorIn1.init());
    ESP_ERROR_CHECK(motorIn2.init());
    ESP_ERROR_CHECK(servoTimer.init());
    ESP_ERROR_CHECK(servo.init());
    //init BLE (nimBLE)

    nvs_flash_init();
    nimble_port_init();
    gatt_svc_init();
    ble_svc_gap_device_name_set("BLECar2");

    ble_hs_cfg.sync_cb = on_sync;
    nimble_port_freertos_init(host_task);
}