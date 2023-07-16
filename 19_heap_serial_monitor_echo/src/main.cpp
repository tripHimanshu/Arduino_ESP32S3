#include <Arduino.h>

// run tasks in core 1 only
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// global variables
static const uint8_t buf_len = 255;
static char *msg_ptr = NULL;
static volatile uint8_t msg_flag = 0;

// Task 1: read message from serial buffer
void read_serial(void *parameter)
{
  char c;
  char buf[buf_len];
  uint8_t idx = 0;
  // clear the memory buffer
  memset(buf, 0, buf_len);
  // run forever
  while (1)
  {
    // read character from serial
    if (Serial.available())
    {
      c = Serial.read();
      // store character to buffer if not over buffer limit
      if (idx < buf_len - 1)
      {
        buf[idx] = c;
        idx++;
      }
      // create message buffer to print task
      if (c == '\n')
      {
        // last character in this string is '\n', so we need to replace it with '\0'
        // to make it null terminated
        buf[idx - 1] = '\0';
        // try to allocate the memory and copy over message
        // if message buffer is still in use, ignore the message
        if (msg_flag == 0)
        {
          msg_ptr = (char *)pvPortMalloc(idx * sizeof(char));
          // if malloc returns 0 (out of memory), throw an error and reset
          configASSERT(msg_ptr);
          // copy the message
          memcpy(msg_ptr, buf, idx);
          // notify other task that message is ready
          msg_flag = 1;
        }
        // reset receive buffer and index counter
        memset(buf, 0, buf_len);
        idx = 0;
      }
    }
  }
}

// Task 2: print message on serial monitor
void print_serial(void *parameter)
{
  // run forever
  while (1)
  {
    // wait for flag to be set and print message
    if (msg_flag == 1)
    {
      Serial.println(msg_ptr);
      // print amount of free heap memory
      Serial.print("Free heap (bytes): ");
      Serial.println(xPortGetFreeHeapSize());
      // free buffer, set pointer to null and clear flag
      vPortFree(msg_ptr);
      msg_ptr = NULL;
      msg_flag = 0;
    }
  }
}

void setup()
{
  // configure serial
  Serial.begin(115200); // baud rate
  // wait for a moment
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--- Serial Monitor Echo ---");
  Serial.println("Enter a string: ");
  // create and run the tasks
  xTaskCreatePinnedToCore(read_serial,    // function to be called
                          "Read Serial",  // task name
                          2048,           // task stack size
                          NULL,           // parameter to pass to function
                          1,              // task priority
                          NULL,           // task handle
                          app_cpu);       // run task in one core
  xTaskCreatePinnedToCore(print_serial,   // function to be called
                          "Print Serial", // task name
                          1024,           // task stack size
                          NULL,           // parameter to pass to function
                          1,              // task priority
                          NULL,           // task handle
                          app_cpu);       // run task in one core

  // delete the setup and loop tasks
  vTaskDelete(NULL);
}

void loop()
{
  // code execution will never reach here
}