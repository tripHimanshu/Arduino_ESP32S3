/*
Heap meory overflow
in vanilla free rtos unless you use heap 3  scheme you want to use pvPortMalloc
as regular malloc is not thread safe
*/
#include <Arduino.h>

// run task in core 1 only
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// task 1: perform some mundane task
void testTask(void *parameter)
{
  // run forever
  while (1)
  {
    int a = 1;
    int b[100]; // array with 100 elements
    // do something with the array so its not optimized by compiler
    for (int i = 0; i < 100; i++)
    {
      b[i] = a + 1;
    }
    Serial.println(b[0]);
    // print remaining stack memory (in words)
    Serial.print("High Water mark (words): ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
    // Print number of free heap memory bytes (before malloc)
    Serial.print("Heap before malloc (bytes): ");
    Serial.println(xPortGetFreeHeapSize());

    int *ptr = (int *)pvPortMalloc(1024 * sizeof(int));

    /*
    one way to prevent heap overflow is to check the malloc output
    uncomment below if else block to show the method to prevent heaap overflow
    */
    if (ptr == NULL)
    {
      Serial.println("Not enough heap");
    }
    else
    {
      // do something with the memory so its not optimized out by compiler
      for (int i = 0; i < 1024; i++)
      {
        ptr[i] = 3;
      }
    }

    /*
    to show the heap overflow uncomment below lines and also
    comment the above if-else block
    */

    // // do something with the memory so its not optimized out by compiler
    // for (int i = 0; i < 1024; i++)
    // {
    //   ptr[i] = 3;
    // }

    // Print number of free heap memory bytes (after malloc)
    Serial.print("Heap after malloc (bytes): ");
    Serial.println(xPortGetFreeHeapSize());

    /*
    to free up the memory use below lines
    */

    // freeup the memory
    vPortFree(ptr);

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  // configure serial
  Serial.begin(115200); // baud rate
  // wait a moment
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--- Memory Overflow ---");
  // create and run the task
  xTaskCreatePinnedToCore(testTask,    // function to be called
                          "Test Task", // task name
                          1500,        // Task stack size
                          NULL,        // parameter to pass to function
                          1,           // task priority
                          NULL,        // task handle
                          app_cpu);    // run task in one core
  // delete setup and loop tasks
  vTaskDelete(NULL);
}

void loop()
{
  // code execution never reaches here
}