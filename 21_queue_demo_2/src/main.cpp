/*
One task performs basic echo on serial
if it sees delay followed by a number, it sends the number
(in a queue) to the second task. if it receives a message
in a second queue, it prints it to the console.
The second task blinks an LED. When it gets a message from the
first queue (number), it updates the blink delay to that number.
When ever the LED blinks 100 times, the second task sends a message
to the first task to be printed
*/

#include <Arduino.h>

// run task in core 1 only
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// settings
static const uint8_t buf_len = 255;     // size of buffer to look for command
static const char command[] = "delay "; // note the space with delay
static const int delay_queue_len = 5;   // size of delay queue
static const int msg_queue_len = 5;     // size of msg queue
static const uint8_t blink_max = 100;   // number times to blink before message

// globals
static QueueHandle_t delay_queue;
static QueueHandle_t msg_queue;

// pins
static const int led_pin = 1;

// message struct
typedef struct Message
{
  char body[20];
  int count;
} Message;

// Task 1: command line interface
void doCLI(void *parameter)
{
  char c;
  char buf[buf_len];
  uint8_t idx;
  uint8_t cmd_len = strlen(command);
  int led_delay;
  Message rcv_msg;
  // clear whole buffer
  memset(buf, 0, buf_len);
  // run forever
  while (1)
  {
    // see if there is a message in the queue (do not block)
    if (xQueueReceive(msg_queue, (void *)&rcv_msg, 0) == pdTRUE)
    {
      Serial.print(rcv_msg.body);
      Serial.println(rcv_msg.count);
    }
    // read character from serial
    if (Serial.available())
    {
      c = Serial.read();
      // store received character to buffer if not over buffer limit
      if (idx < buf_len - 1)
      {
        buf[idx] = c;
        idx++;
      }
      // print newline and check input on enter
      if ((c == '\n') || (c == '\r'))
      {
        // print newline to terminal
        Serial.print("\r\n");
        // check if the first 6 characters are "delay "
        if (memcmp(buf, command, cmd_len) == 0)
        {
          // convert last part to positive integer (negative int crashes)
          char *tail = buf + cmd_len;
          led_delay = atoi(tail);
          led_delay = abs(led_delay);
          // send integer to other task via queue
          if (xQueueSend(delay_queue, (void *)&led_delay, 10) != pdTRUE)
          {
            Serial.println("ERROR: Could not put item on delay queue");
          }
        }
        // reset receive buffer and index counter
        memset(buf, 0, buf_len);
        idx = 0;
      }
      else
      {
        Serial.print(c); // echo the character back to serial terminal
      }
    }
  }
}

// Task 2: flash led based on delay provided, notify other task every 100 blinks
void blinkLED(void *parameter)
{
  Message msg;
  int led_delay = 500;
  uint8_t counter = 0;
  // setup the pin
  pinMode(led_pin, OUTPUT);
  // run forever
  while (1)
  {
    // see it there is a message in queue(do not block)
    if (xQueueReceive(delay_queue, (void *)&led_delay, 0) == pdTRUE)
    {
      // use only one task to manage serial comms (best practice)
      strcpy(msg.body, "message received");
      msg.count = 1;
      xQueueSend(msg_queue, (void *)&msg, 10);
    }
    // led blink
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    // if blinked 100 times, send a message to the other task
    counter++;
    if (counter >= blink_max)
    {
      // construct message and send
      strcpy(msg.body, "Blinked: ");
      msg.count = counter;
      xQueueSend(msg_queue, (void *)&msg, 10);
      // reset counter
      counter = 0;
    }
  }
}

void setup()
{
  // configure serial
  Serial.begin(115200);                  // baud rate
  vTaskDelay(1000 / portTICK_PERIOD_MS); // wait for a moment
  Serial.println();
  Serial.println("--- Queue demo 2 ---");
  Serial.println("Enter the command 'delay xxx' where xxx is delay in ms");
  // create queues
  delay_queue = xQueueCreate(delay_queue_len, sizeof(int));
  msg_queue = xQueueCreate(msg_queue_len, sizeof(Message));
  // create and run tasks
  xTaskCreatePinnedToCore(doCLI,       // function to be called
                          "Do CLI",    // task name
                          2048,        // taks stack size
                          NULL,        // parameter to pass to function
                          1,           // task priority
                          NULL,        // task handle
                          app_cpu);    // run task in core one
  xTaskCreatePinnedToCore(blinkLED,    // function to be called
                          "blink led", // task name
                          2048,        // taks stack size
                          NULL,        // parameter to pass to function
                          1,           // task priority
                          NULL,        // task handle
                          app_cpu);    // run task in core one

  // delete setup and loop tasks
  vTaskDelete(NULL);
}

void loop()
{
  // code execution never reaches here
}