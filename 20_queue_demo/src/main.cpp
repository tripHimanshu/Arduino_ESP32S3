/*
Board: ESP32S3 devkitC-1 v1.0
Framework: Arduino & FreeRTOS
Author: Himanshu Tripathi
Description:
Queue demo with FreeRTOS
to understand the queue operations follow the following steps
1. write in queue and read from queue in every 1 second
2. write in queue in each 500 ms and read from queue in each 1 second
3. write in queue in each 1 second and read from queue in each 500 ms
Default the code is configured in case 1
*/
#include <Arduino.h>

// run tasks in core 1 only
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// globals
static const uint8_t msg_queue_len = 5;
static QueueHandle_t msg_queue;

// Task: Read item from queue and print it
void print_task(void *parameter)
{
  int item;
  // run forever
  while (1)
  {
    // see if there is any item in queue (do not block)
    if (xQueueReceive(msg_queue, (void *)&item, 0) == pdTRUE)
    {
      /* uncomment the below line for case 1 and 2
      comment the below line for case 3*/
      Serial.println(item);
    }
    /*uncomment the below line for case 3
    comment the below line for case 1 and 2*/
    // Serial.println(item);
    // wait for trying again
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  // configure serial
  Serial.begin(115200);                  // baud rate
  vTaskDelay(1000 / portTICK_PERIOD_MS); // wait for a momemt
  Serial.println();
  Serial.println("--- FreeRTOS Queue demo ---");

  // create queue
  msg_queue = xQueueCreate(msg_queue_len, sizeof(int));

  // create and run task
  xTaskCreatePinnedToCore(print_task,   // function to be called
                          "print task", // task name
                          1024,         // task stack size
                          NULL,         // parameter to pass to function
                          1,            // task priority
                          NULL,         // task handle
                          app_cpu);     // run task in one core
}

void loop()
{
  static int num = 0;
  // try to add item in queue for 10 ticks, fail if queue is full
  if (xQueueSend(msg_queue, (void *)&num, 10) != pdTRUE)
  {
    Serial.println("Queue full");
  }
  num++;
  // wait before trying again
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}