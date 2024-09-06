#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "h/task_comms.h"
#include "h/task_sensors.h"
#include "freertos/FreeRTOS.h"

const static char *TAG = "main";

#define TASK_PRIO_3         3
#define CORE0       0
#define CORE1       ((CONFIG_FREERTOS_NUMBER_OF_CORES > 1) ? 1 : tskNO_AFFINITY)


void app_main(void)
{
    static QueueHandle_t msg_queue;
    static const uint8_t msg_queue_len = 40;

    msg_queue = xQueueGenericCreate(msg_queue_len, sizeof(int), queueQUEUE_TYPE_SET);
    if (msg_queue == NULL) {
        ESP_LOGE(TAG, "Error creating queue. Stopping!");
        return;
    }

    xTaskCreatePinnedToCore(task_sensors, "core0_sensors", 4096, (void*)&msg_queue, TASK_PRIO_3, NULL, CORE0);
    xTaskCreatePinnedToCore(task_comms, "core1_comms", 4096, (void*)&msg_queue, TASK_PRIO_3, NULL, CORE1);
   

}