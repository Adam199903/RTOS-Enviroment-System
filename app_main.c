/*
 * app_main.c
 *
 * this is where we create the main application thread(s) and kick everything
 * off
 *
 * code used comes from a combination of lab 4 part 3 and part 4
 * 
 * 
 */

// include functions from the c standard libraries
#include <stdint.h>
#include <stdio.h>

// include cmsis-rtos version 2
#include "cmsis_os2.h"

// include my gpio library
#include "pinmappings.h"
#include "gpio.h"

// include my rtos objects
#include "rtos_objects.h"

// DEFINES

// network initialisation function prototype is defined elsewhere
extern int32_t socket_startup(void);

//push button 1 (on bread board)
gpio_pin_t pb1 = {PI_3, GPIOI, GPIO_PIN_3};
//push button 2, user blue button on stm
gpio_pin_t pb2 = {PI_11, GPIOI, GPIO_PIN_11};

// RTOS DEFINES
//---------------------------- enviroment datathread--------------------
osThreadId_t enviroment_thread;
extern void enviroment_data(void *argument);
static const osThreadAttr_t env_data_thread_attr =
{
  .name = "enviroment_data",
  .priority = osPriorityNormal,
};

//-------------------------------- display thread-------------------------
osThreadId_t display_thread;
extern void lcd_data_display(void *argument);
static const osThreadAttr_t display_thread_attr =
{
  .name = "lcd_display",
  .priority = osPriorityNormal,
};

//----------------------- mqtt thread (with increased stack size)----------------
osThreadId_t mqtt_thread;
extern void mqtt_run_task(void *argument);
static const osThreadAttr_t mqtt_thread_attr =
{
  .name = "mqtt_thread",
  .priority = osPriorityNormal,
  .stack_size = 8192U,
};

//----------------------------------- my message queue-------------------------
osMessageQueueId_t m_messages;
static const osMessageQueueAttr_t msq_q_attr =
{
  .name = "my_messages",
};

// --------------------declare a timer callback and a timer---------------------
//for button 1(lights)
void test_for_button_press(void * parameters);
osTimerId_t button;
static const osTimerAttr_t button_timer =
{
  .name = "button_debouncing_timer",
};

//for button 2 (security)
void test_for_button_press2(void * parameters2);
osTimerId_t button2;
static const osTimerAttr_t button_timer2 =
{
  .name = "button_debouncing_timer2",
};


// event flag
// ----------------------declare button pressed event flag-----------------
//for button 1(lights)
osEventFlagsId_t button_flag;
static const osEventFlagsAttr_t button_flag_attr =
{
  .name = "button_event",
};

//for button 2 (security)
osEventFlagsId_t button_flag2;
static const osEventFlagsAttr_t button_flag_attr2 =
{
  .name = "button_event2",
};


// --------------------------------MAIN THREAD-----------------------------
// provide an increased stack size to the main thread
static const osThreadAttr_t app_main_attr =
{
  .stack_size = 8192U
};

// main thread
static void app_main(void *argument)
{

  // set up the network connection
  int32_t status = socket_startup();
	
	init_gpio(pb1, INPUT);
	button_flag = osEventFlagsNew(&button_flag_attr);
	
	init_gpio(pb2, INPUT);
  button_flag2 = osEventFlagsNew(&button_flag_attr2);

  // create the timer object for button debouncing and start it
  button = osTimerNew(&test_for_button_press, osTimerPeriodic, NULL, &button_timer);
  osTimerStart(button, 5);
	
  button2 = osTimerNew(&test_for_button_press2, osTimerPeriodic, NULL, &button_timer2);
  osTimerStart(button2, 5);
	
  // create the message queue
  m_messages = osMessageQueueNew(16, sizeof(message_t), &msq_q_attr);
	
  // if the network connected ok then create the mqtt thread to send and
  // receive packets
  if(status == 0)
  {
		enviroment_thread  = osThreadNew(enviroment_data, NULL, &env_data_thread_attr);
		display_thread = osThreadNew(lcd_data_display, NULL, &display_thread_attr);
    mqtt_thread = osThreadNew(mqtt_run_task, NULL, &mqtt_thread_attr);
  }
}

//-------------------------------------BUTTON DEBOUNCING--------------------------------
//-----------------------------------Light switch debouncing--------------------------
// button debouncing using pattern matching (implemented as a timer callback)
void test_for_button_press(void * parameters)
{
  // 8 bits of button history
  static uint8_t button_history = 0xFF;

  // every time this timer callback is called we shift the button history
  // across and update the state
  button_history = button_history << 1;
  uint8_t val = read_gpio(pb1);
  button_history = button_history | val;

  // use some simple pattern matching to see if the button has been pressed 
  // and released - if so, reset the button history and set the event flag ...
  if((button_history & 0xC7) == 0x07)
  {
    // reset button history
    button_history = 0xFF;

    // signal the button has been pressed
    osEventFlagsSet(button_flag, 0x01);
  }
}
//---------------------------------------security switch debouncing---------------------------------
void test_for_button_press2(void * parameters2)
{
  // 8 bits of button history
  static uint8_t button_history2 = 0xFF;

  // every time this timer callback is called we shift the button history
  // across and update the state
  button_history2 = button_history2 << 1;
  uint8_t val2 = read_gpio(pb2);
  button_history2 = button_history2 | val2;

  // use some simple pattern matching to see if the button has been pressed 
  // and released - if so, reset the button history and set the event flag ...
  if((button_history2 & 0xC7) == 0x07)
  {
    // reset button history
    button_history2 = 0xFF;

    // signal the button has been pressed
    osEventFlagsSet(button_flag2, 0x01);
  }
}

// THREAD INITIALISATION

// initialise the application (by creating the main thread)
void app_initialize(void)
{
  osThreadNew(app_main, NULL, &app_main_attr);
}
