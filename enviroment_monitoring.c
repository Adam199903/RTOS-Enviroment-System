// include the c standard io library
#include <stdio.h>

// include cmsis_os for the rtos api
#include "cmsis_os2.h"

// include my rtos objects
#include "rtos_objects.h"

// include the shu bsp libraries for the stm32f7 discovery board
#include "pinmappings.h"
#include "gpio.h"
#include "adc.h"


// HARDWARE DEFINES
gpio_pin_t led1 = {PI_1, GPIOI, GPIO_PIN_1};

// sensors
gpio_pin_t ldr = {PA_0,  GPIOA, GPIO_PIN_0};
gpio_pin_t temp_pot = {PF_9, GPIOF, GPIO_PIN_9};
gpio_pin_t thermostat = {PF_6, GPIOF, GPIO_PIN_6};

//IR sesnor
gpio_pin_t IR = {PG_7, GPIOG, GPIO_PIN_7};

// ACTUAL THREAD

// data acquisition thread - read the adcs and stuff the data in a message
// queue
void enviroment_data(void const *argument)
{
  // print a status message
  printf("enviroment monitoring thread up and running ...\r\n");

	//LED 
	init_gpio(led1, OUTPUT);
	
  // set up the adc for the temperature and light sensor
  init_adc(ldr);
	init_adc(temp_pot);
	init_adc(thermostat);
	
  //initalise the IR sensor
	init_gpio(IR, INPUT);

  // initialise a message container
  message_t msg;

  // infinite loop generating our fake data (one set of samples per second)
  // we also toggle the led so we can see what is going on ...
  while(1)
  {
    // read the sensors
    uint16_t adcLDR = read_adc(ldr);
		uint16_t adcTemp = read_adc(temp_pot);
		uint16_t adcThermostat = read_adc(thermostat);
	
		
    // put our data in the message object
    msg.LDR = adcLDR;
		msg.adctemp = adcTemp; // we read the adc value of the temp sens so we can check that the temp value we get is correct
		msg.Temp = ((adcTemp * (5 / 4095.0)) - 0.5) * 1000 / 10.0;
		msg.Thermostat = (adcThermostat / 135.0) + 5; //range of 5-35C
		msg.PIR = read_gpio(IR);
		
//----------------------------------Thermostat---------------------------------------		
		//turns on led(acts as heaters) if temp is lower then thermostate level, range of 5-35C
		if( msg.Temp < msg.Thermostat){
			write_gpio(led1, HIGH);
		}
		else{
			write_gpio(led1, LOW);
		}

//------------------put the data in the message queue and wait for 3 seconds-----------------
    osMessageQueuePut(m_messages, &msg, osPriorityNormal, osWaitForever);
    osDelay(3000);
  }
}


