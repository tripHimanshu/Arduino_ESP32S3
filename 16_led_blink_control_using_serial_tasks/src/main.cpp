#include <Arduino.h>
#include <stdlib.h>

// run tasks in core 1 only
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// pins
static const int led_pin = 1;

// global variables
static const uint8_t buf_len = 20;
static int blink_interval = 500; // ms

// Task 1: Blink LED at rate set by global variable
void toggleLED(void *parameter)
{
  // run forever
  while (1)
  {
    digitalWrite(led_pin, HIGH); // led is on
    vTaskDelay(blink_interval / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW); // led is off
    vTaskDelay(blink_interval / portTICK_PERIOD_MS);
  }
}

// Task 2: read from serial terminal
void readSerial(void *parameter)
{
  char c;
  char buf[buf_len];
  uint8_t idx = 0;
  memset(buf, 0, buf_len); // clear whole buffer
  // run forever
  while (1)
  {
    // read characters from serial
    if (Serial.available())
    {
      c = Serial.read();
      // update delay variable and reset buffer if we get a newine character
      if (c == '\n')
      {
        blink_interval = atoi(buf);
        Serial.print("Updated LED blink interval to ");
        Serial.println(blink_interval);
        memset(buf, 0, buf_len);
        idx = 0;
      }
      else
      {
        // only append if index is not over message limit
        if (idx < buf_len - 1)
        {
          buf[idx] = c;
          idx++;
        }
      }
    }
  }
}

void setup()
{
  // configure pin
  pinMode(led_pin, OUTPUT);
  // configure serial and wait a second
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("Enter the new blink interval (in ms): ");
  // create and run the task
  xTaskCreatePinnedToCore(toggleLED,     // function to be called
                          "LED Toggle",  // task name
                          1024,          // task stack size
                          NULL,          // parameter to pass to function
                          1,             // task priority
                          NULL,          // task handle
                          app_cpu);      // run task in one core
  xTaskCreatePinnedToCore(readSerial,    // function to be called
                          "Read Serial", // task name
                          2048,          // task stack size
                          NULL,          // parameter to pass to function
                          1,             // task priority
                          NULL,          // task handle
                          app_cpu);      // run task in one core

  // delete setup and loop tasks
  vTaskDelete(NULL);
}

void loop()
{
  // code execution will never get here
}
