#include <Arduino.h>

// run tasks in core 1 only
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// message
const char msg[] = "This is sample message";

// task handles
static TaskHandle_t task1 = NULL;
static TaskHandle_t task2 = NULL;

// Task 1: Print message to serial terminal with lower priority
void print_msg(void *parameter)
{
  // count the number of character in message
  int msg_len = strlen(msg);
  // run forever
  while (1)
  {
    Serial.println();
    for (int i = 0; i < msg_len; i++)
    {
      Serial.print(msg[i]);
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    Serial.println();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Task 2: Print star to serial monitor with higher priority
void print_star(void *parameter)
{
  // run forever
  while (1)
  {
    Serial.print("*");
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  // configure serial
  Serial.begin(300);                     // baud rate
  vTaskDelay(1000 / portTICK_PERIOD_MS); // wait
  Serial.println();
  Serial.println("---Task Pre-emptive Scheduling---");
  Serial.println();
  // print self priority and core
  Serial.print("Setup and Loop tasks are running in core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));

  // Create and run tasks
  xTaskCreatePinnedToCore(print_msg,   // function to be called
                          "print msg", // task name
                          1024,        // task stack size
                          NULL,        // parameter to pass to function
                          1,           // task priority
                          &task1,      // task handle
                          app_cpu);    // run task in one core

  xTaskCreatePinnedToCore(print_star,   // function to be called
                          "print star", // task name
                          1024,         // task stack size
                          NULL,         // parameter to pass to function
                          2,            // task priority
                          &task2,       // task handle
                          app_cpu);     // run task in one core
}

void loop()
{
  // suspend the higher priority task for some interval
  for (int i = 0; i < 4; i++)
  {
    vTaskSuspend(task2);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    vTaskResume(task2);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  // delete the lower priority task
  // before deleting the task make sure that the task handle is not NULL
  // and after task deletion make the task handle NULL
  if (task1 != NULL)
  {
    vTaskDelete(task1);
    task1 = NULL;
  }
}