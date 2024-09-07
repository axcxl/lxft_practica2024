#include <string.h>

#include "esp_log.h"
#include "esp_err.h"
#include "bmp280.h"
#include "freertos/FreeRTOS.h"
#include "h/sensor_queue.h"
#include "h/task_sensors.h"

const static char *TAG = "sensors";

/*  NOTE: we have a BME280 sensor on board, but the
 *  driver is for both the BME and BMP. Will use BME280 
 *  all functions here, it is not a mistake.
 */
bmp280_t *init_bme280()
{
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    bmp280_t *dev = (bmp280_t*)malloc(sizeof(bmp280_t));
    memset(dev, 0, sizeof(bmp280_t));

    /* On our boards, BME280 address is 0x77 */
    /* For I2C pins check the UEXT connector */
    ESP_ERROR_CHECK(bmp280_init_desc(dev, BMP280_I2C_ADDRESS_1, 0, GPIO_NUM_13, GPIO_NUM_16));
    ESP_ERROR_CHECK(bmp280_init(dev, &params));

    bool bme280p = dev->id == BME280_CHIP_ID;
    printf("BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

    return dev;
}

void read_send_bme280(bmp280_t *dev, QueueHandle_t* queue)
{
    float pressure, temperature, humidity;
    sensq to_send;

    /* Read all info from sensor */
    if (bmp280_read_float(dev, &temperature, &pressure, &humidity) != ESP_OK)
    {
        ESP_LOGE(TAG, "Temperature/pressure reading failed");
        return;
    }

    /* Put it in the queue one at a time */
    to_send.value = temperature;
    to_send.type = TEMP;
    if (xQueueGenericSend(*(QueueHandle_t*)queue, (void *)&to_send, portMAX_DELAY, queueSEND_TO_BACK) != pdTRUE) 
    {
        ESP_LOGE(TAG, "Queue full");
    }

    to_send.value = pressure;
    to_send.type = PRES;
    if (xQueueGenericSend(*(QueueHandle_t*)queue, (void *)&to_send, portMAX_DELAY, queueSEND_TO_BACK) != pdTRUE) 
    {
        ESP_LOGE(TAG, "Queue full");
    }

    to_send.value = humidity;
    to_send.type = HUM;
    if (xQueueGenericSend(*(QueueHandle_t*)queue, (void *)&to_send, portMAX_DELAY, queueSEND_TO_BACK) != pdTRUE) 
    {
        ESP_LOGE(TAG, "Queue full");
    }

}

void task_sensors(void* msg_queue)
{ 
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xTimeIncrement = pdMS_TO_TICKS(10000); // run every 10 second
    bmp280_t *dev_bme280;

    ESP_ERROR_CHECK(i2cdev_init());
    dev_bme280 = init_bme280();
    
    while(1){
        vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);

        read_send_bme280(dev_bme280, msg_queue);
    }
}