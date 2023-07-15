#include <Arduino.h>

// run tasks in core 1 only
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// pins definition
static const int led_pin = 16;

// Task 1: blink led with 500 ms delay
void task1(void *parameter)
{
  // run forever
  while (1)
  {
    digitalWrite(led_pin, HIGH);          // turn on the led
    vTaskDelay(500 / portTICK_PERIOD_MS); // delay
    digitalWrite(led_pin, LOW);           // turn off the led
    vTaskDelay(500 / portTICK_PERIOD_MS); // delay
  }
}

// Task 2: blink led with 700 ms delay
void task2(void *parameter)
{
  // run forever
  while (1)
  {
    digitalWrite(led_pin, LOW);           // turn off the led
    vTaskDelay(700 / portTICK_PERIOD_MS); // delay
    digitalWrite(led_pin, HIGH);          // turn on the led
    vTaskDelay(700 / portTICK_PERIOD_MS); // delay
  }
}

void setup()
{
  // pins configuration
  pinMode(led_pin, OUTPUT);
  // create and run task 1
  xTaskCreatePinnedToCore(task1,    // function to be called
                          "Task 1", // task name
                          1024,     // task stack size
                          NULL,     // parameter to pass to function
                          1,        // task priority
                          NULL,     // task handle
                          app_cpu); // run task in one core
  // create and run task 2
  xTaskCreatePinnedToCore(task2,    // function to be called
                          "Task 2", // task name
                          1024,     // task stack size
                          NULL,     // parameter to pass to function
                          1,        // task priority
                          NULL,     // task handle
                          app_cpu); // run task in one core
}

void loop()
{
  // no code is needed here
}