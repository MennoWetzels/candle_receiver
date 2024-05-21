#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"

#include "flame.hpp"

#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA

extern "C"
{
  void app_main(void);
}

static const char *TAG = "espnow_example";

typedef struct candle_data
{
   uint8_t command;
   uint8_t data;
} candle_data_t;

candle_data_t received_data;

enum class CANDLE_COMMAND
{
	BRIGHTNESS 	= 0x01,
	ALPHA		= 0x02,
	RESTART	    = 0x0F,
};

flame flame1;

/* WiFi should start before using ESPNOW */
static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());

    // enable esp now long range
    //ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );

}

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    memcpy(&received_data, data, sizeof(received_data));
    ESP_LOGI(TAG, "ESP Data recieved: %d | %d", received_data.command, received_data.data);

    switch (received_data.command){
      case ((uint8_t)CANDLE_COMMAND::BRIGHTNESS):
      {
        flame1.adjust_brightness((uint8_t)received_data.data);
        break;
      }
      case ((uint8_t)CANDLE_COMMAND::ALPHA):
      {
        flame1.adjust_alpha((uint8_t)received_data.data);
        break;
      }
    }
}

static esp_err_t espnow_init(void)
{
    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_recv_cb((esp_now_recv_cb_t)espnow_recv_cb) );
    return ESP_OK;
}

void flicker_task(void *arg)
{
  init_led_controller();
  flame1.setup(LED1_IO, LED1_CH, 20, 100);

  for (;;)
  {
    flame1.flicker();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    wifi_init();
    espnow_init();

    xTaskCreate(flicker_task, "flicker_task", 4096, NULL, 2, NULL);
}