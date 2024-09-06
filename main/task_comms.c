#include "esp_log.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include "ethernet_init.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "mqtt_client.h"
#include "h/task_comms.h"

static esp_eth_handle_t *s_eth_handles = NULL;
static uint8_t s_eth_port_cnt = 0;
const static char *TAG = "comms";

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

void task_comms(void* msg_queue)
{
    char mqttdata[10];
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
        if (xQueueReceive(*(QueueHandle_t*)msg_queue, (void *)&data, xTicksToWait) == pdTRUE) 
        {
            sprintf(mqttdata,"%d",data);
            ESP_LOGI(TAG, "received data = %d", data);
            esp_mqtt_client_publish(client, "/topic/sensor3", mqttdata, 0, 0, 0);
        } 
    }
}