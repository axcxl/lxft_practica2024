#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "h/task_sensors.h"

const static char *TAG = "sensors";

void task_sensors(void* msg_queue)
{ 
    TickType_t xLastWakeTime;
    const TickType_t xTimeIncrement = pdMS_TO_TICKS(1000); // run every 1 second
    int sent_num  = 0;
       const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    while(1){
        vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
        if (xQueueGenericSend(*(QueueHandle_t*)msg_queue, (void *)&sent_num, portMAX_DELAY, queueSEND_TO_BACK) != pdTRUE) 
        {
            ESP_LOGI(TAG, "Queue full\n");
        }
        vTaskDelay(xDelay);
        sent_num++;
        }
}