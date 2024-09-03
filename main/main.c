#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

static QueueHandle_t msg_queue;
static const uint8_t msg_queue_len = 40;
const static char *TAG = "queue example";

#define TASK_PRIO_3         3
#define CORE0       0
#define CORE1       ((CONFIG_FREERTOS_NUMBER_OF_CORES > 1) ? 1 : tskNO_AFFINITY)
#define QUEUE_CREATE_ERR_STR              "queue creation failed"

void core_i2c(void* arg){ 
    TickType_t xLastWakeTime;
    const TickType_t xTimeIncrement = pdMS_TO_TICKS(1000); // run every 1 second
    int sent_num  = 0;
    while(1){
        vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
        if (xQueueGenericSend(msg_queue, (void *)&sent_num, portMAX_DELAY, queueSEND_TO_BACK) != pdTRUE) 
        {
            ESP_LOGI(TAG, "Queue full\n");
        }
        sent_num++;
        }
}
void core_comm(void* arg){
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    int data;  // data type should be same as queue item type
    int to_wait_ms = 100;  // the maximal blocking waiting time of millisecond
    const TickType_t xTicksToWait = pdMS_TO_TICKS(to_wait_ms);

    while(1){
        if (xQueueReceive(msg_queue, (void *)&data, xTicksToWait) == pdTRUE) 
        {
            ESP_LOGI(TAG, "received data = %d", data);
        } 
        vTaskDelay(xDelay);
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