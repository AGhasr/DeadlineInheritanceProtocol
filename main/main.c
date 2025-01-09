#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_attr.h"
#include "esp_cpu.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

// Create two mutexes
static SemaphoreHandle_t mutex1;
static SemaphoreHandle_t mutex2;
static SemaphoreHandle_t mutex3;

TaskHandle_t xTaskLHandleL = NULL;  // Declare the TaskHandle_t variable


// Task L (low priority)
void doTaskL(void *parameters) {
    TickType_t timestamp;
    TickType_t startTime, endTime, executionTime;
    while (1) {
        if (xSemaphoreTake(mutex1, portMAX_DELAY) == pdTRUE) {
            printf("Task L acquired the lock1. Doing some work...\n");
            vLogReadyListOrder();
            // Record the start time
            startTime = xTaskGetTickCount();

            if (xSemaphoreTake(mutex2, portMAX_DELAY) == pdTRUE) {
                printf("Task L acquired lock2. Doing some work...\n");
                vLogReadyListOrder();

                if (xSemaphoreTake(mutex3, portMAX_DELAY) == pdTRUE) {
                    printf("Task L acquired lock3. Doing some work...\n");
                    vLogReadyListOrder();

                    // Simulate work
                    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < 1000);

                    

                    printf("Task L is going to release lock3...\n");
                    vLogReadyListOrder();

                    xSemaphoreGive(mutex3);
                    
                }
                printf("Task L continue its work with lock2...\n");
                // Simulate work
                timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
                while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < 1000);

                

                printf("Task L is going to release lock2...\n");
                vLogReadyListOrder();

                xSemaphoreGive(mutex2);

                
            }
            printf("Task L continue its work with lock1...\n");
            vLogReadyListOrder();
            // Simulate work
            timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
            while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < 1000);

            // Record the end time
            endTime = xTaskGetTickCount();

            // Calculate the execution time in milliseconds
            executionTime = (endTime - startTime) * portTICK_PERIOD_MS;
            
            printf("Task L took %lu ms to complete its work.\n", (unsigned long)executionTime);
        
            printf("Task L  will release lock1 now...\n");
            vLogReadyListOrder();
            xSemaphoreGive(mutex1);
            printf("Task L released lock1...\n");
        }

        printf("Task L Done!\n");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Task M (medium priority)
void doTaskM(void *parameters) {
    TickType_t timestamp;
    vTaskDelay(pdMS_TO_TICKS(50));

    while (1) {
        printf("Task M trying to take lock1...\n");

        vLogReadyListOrder();
        // Acquire lock2
        if (xSemaphoreTake(mutex1, portMAX_DELAY) == pdTRUE) {
            printf("Task M acquired lock1. Doing some work...\n");
            vLogReadyListOrder();

            // Simulate work
            timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
            while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < 1000);

            printf("Task M released lock1...\n");
            xSemaphoreGive(mutex1);
            
        }
        printf("Task M Done!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task H (high priority)
void doTaskH(void *parameters) {
    TickType_t timestamp;
    vTaskDelay(pdMS_TO_TICKS(100));
    while (1) {
        printf("Task H trying to take lock2...\n");

        vLogReadyListOrder();
        // Acquire lock2
        if (xSemaphoreTake(mutex2, portMAX_DELAY) == pdTRUE) {
            printf("Task H acquired lock2. Doing some work...\n");
            
            vLogReadyListOrder();

            // Simulate work
            timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
            while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < 1000);

            printf("Task H released lock2...\n");
            xSemaphoreGive(mutex2);
            
        }
        printf("Task H Done!\n");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// Task HH (highest priority)
void doTaskHH(void *parameters) {
    TickType_t timestamp;
    vTaskDelay(pdMS_TO_TICKS(150));
    while (1) {
        printf("Task HH trying to take lock3...\n");

        vLogReadyListOrder();
        // Acquire lock3
        if (xSemaphoreTake(mutex3, portMAX_DELAY) == pdTRUE) {
            printf("Task HH acquired lock3. Doing some work...\n");
            
            vLogReadyListOrder();

            // Simulate work
            timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
            while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < 1000);

            printf("Task HH released lock3...\n");
            xSemaphoreGive(mutex3);
            
        }
        printf("Task HH Done!\n");
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

void app_main() {
    printf("---FreeRTOS Deadline Inheritance Demo---\n");

    // Create two mutexes
    mutex1 = xSemaphoreCreateMutex();
    mutex2 = xSemaphoreCreateMutex();
    mutex3 = xSemaphoreCreateMutex();

    xTaskCreate(doTaskL, "Task L", 2048, NULL, 0, NULL, pdMS_TO_TICKS(1000));
    xTaskCreate(doTaskM, "Task M", 2048, NULL, 0, NULL, pdMS_TO_TICKS(900));
    xTaskCreate(doTaskH, "Task H", 2048, NULL, 0, NULL, pdMS_TO_TICKS(800));
    xTaskCreate(doTaskHH, "Task HH", 2048, NULL, 0, NULL, pdMS_TO_TICKS(700));

    ESP_LOGI("app_main", "Starting scheduler from app_main()");
    vTaskStartScheduler();

    ESP_LOGE("app_main", "insufficient RAM! aborting");
    abort();
}
