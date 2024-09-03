#include <stdio.h>
#include "freertos/FreeRTOS.h"

#define TASK_PRIO_3         3
#define CORE0       0
#define CORE1       ((CONFIG_FREERTOS_NUMBER_OF_CORES > 1) ? 1 : tskNO_AFFINITY)


void core_i2c(void* arg){ 
    const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
    while(1){
        vTaskDelay(xDelay);
        printf("Core I2C");}
}
void core_comm(void* arg){
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    while(1){
        vTaskDelay(xDelay);
        printf("Core Comm");}
}

void app_main(void)
{
    xTaskCreatePinnedToCore(core_i2c, "pinned_task1_core0", 4096, NULL, TASK_PRIO_3, NULL, CORE0);
    xTaskCreatePinnedToCore(core_comm, "pinned_task2_core1", 4096, NULL, TASK_PRIO_3, NULL, CORE1);



}