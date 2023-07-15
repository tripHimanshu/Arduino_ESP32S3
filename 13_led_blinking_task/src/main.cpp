#include <Arduino.h>
// use only core 1 for running the task
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// pins configurations
static const int led_pin = 16;

// task: blink the led
void led_blink(void *parameter)
{
  // run forever
  while (1)
  {
    digitalWrite(led_pin, HIGH);           // turn on the led
    vTaskDelay(1000 / portTICK_PERIOD_MS); // delay
    digitalWrite(led_pin, LOW);            // turn off the led
    vTaskDelay(1000 / portTICK_PERIOD_MS); // delay
  }
}

void setup()
{
  // pin configuration
  pinMode(led_pin, OUTPUT);
  // create and run the task
  xTaskCreatePinnedToCore(led_blink,   // function to be called
                          "Blink LED", // task name
                          1024,        // task stack size
                          NULL,        // parameter to pass to function
                          1,           // task priority
                          NULL,        // task handle
                          app_cpu);    // run task in one core
}

void loop()
{
  // no code is needed here
}