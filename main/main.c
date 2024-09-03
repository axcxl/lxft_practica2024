#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include <string.h>
#include "ethernet_init.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "mqtt_client.h"

static QueueHandle_t msg_queue;
static const uint8_t msg_queue_len = 40;
const static char *TAG = "queue example";
static esp_eth_handle_t *s_eth_handles = NULL;
static uint8_t s_eth_port_cnt = 0;

#define TASK_PRIO_3         3
#define CORE0       0
#define CORE1       ((CONFIG_FREERTOS_NUMBER_OF_CORES > 1) ? 1 : tskNO_AFFINITY)
#define QUEUE_CREATE_ERR_STR              "queue creation failed"

void init_ethernet_and_netif(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_eth_init(&s_eth_handles, &s_eth_port_cnt));

    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
    esp_netif_config_t cfg_spi = {
        .base = &esp_netif_config,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
    };
    char if_key_str[10];
    char if_desc_str[10];
    char num_str[3];
    for (int i = 0; i < s_eth_port_cnt; i++) {
        itoa(i, num_str, 10);
        strcat(strcpy(if_key_str, "ETH_"), num_str);
        strcat(strcpy(if_desc_str, "eth"), num_str);
        esp_netif_config.if_key = if_key_str;
        esp_netif_config.if_desc = if_desc_str;
        esp_netif_config.route_prio -= i*5;
        esp_netif_t *eth_netif = esp_netif_new(&cfg_spi);

        // attach Ethernet driver to TCP/IP stack
        ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(s_eth_handles[i])));
    }

    for (int i = 0; i < s_eth_port_cnt; i++) {
        ESP_ERROR_CHECK(esp_eth_start(s_eth_handles[i]));
    }
}

void core_i2c(void* arg){ 
    TickType_t xLastWakeTime;
    const TickType_t xTimeIncrement = pdMS_TO_TICKS(1000); // run every 1 second
    int sent_num  = 0;
       const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    while(1){
        vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
        if (xQueueGenericSend(msg_queue, (void *)&sent_num, portMAX_DELAY, queueSEND_TO_BACK) != pdTRUE) 
        {
            ESP_LOGI(TAG, "Queue full\n");
        }
        vTaskDelay(xDelay);
        sent_num++;
        }
}
void core_comm(void* arg){
    char mqttdata[10];

    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    int data;  // data type should be same as queue item type
    int to_wait_ms = 100;  // the maximal blocking waiting time of millisecond
    const TickType_t xTicksToWait = pdMS_TO_TICKS(to_wait_ms);
     esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };
    init_ethernet_and_netif();
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
    while(1){
        if (xQueueReceive(msg_queue, (void *)&data, xTicksToWait) == pdTRUE) 
        {
            sprintf(mqttdata,"%d",data);
            ESP_LOGI(TAG, "received data = %d", data);
            esp_mqtt_client_publish(client, "/topic/sensor3", mqttdata, 0, 0, 0);
        } 
        }
}

void app_main(void)
{
    msg_queue = xQueueGenericCreate(msg_queue_len, sizeof(int), queueQUEUE_TYPE_SET);
    if (msg_queue == NULL) {
        ESP_LOGE(TAG, QUEUE_CREATE_ERR_STR);
        return;
    }

    xTaskCreatePinnedToCore(core_i2c, "pinned_task1_core0", 4096, NULL, TASK_PRIO_3, NULL, CORE0);
    xTaskCreatePinnedToCore(core_comm, "pinned_task2_core1", 4096, NULL, TASK_PRIO_3, NULL, CORE1);
   

}