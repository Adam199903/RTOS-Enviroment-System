// include the c standard io library
#include <stdio.h>
#include "stm32f7xx_hal.h"

// include cmsis_os for the rtos api
#include "cmsis_os2.h"

// include my rtos objects
#include "rtos_objects.h"

// include the shu bsp libraries for the stm32f7 discovery board
#include "pinmappings.h"
#include "clock.h"
#include "random_numbers.h"
#include "gpio.h"
#include "stm32746g_discovery_lcd.h"
#include "adc.h"

//LCD defines

#define BOARDER	"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

//HEADER MESSAGE

const char * header_message[2] = 
{
	" HELLO THIS IS YOUR ENVIRONMENT",
	"      MONITORING SYSTEM       "
	
};

void LCDdisplay(void)
{
	
	// initialise the lcd
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, SDRAM_DEVICE_ADDR);
  BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

  // set the background colour to black and clear the lcd
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_Clear(LCD_COLOR_BLACK);
  
  // set the font to use
  BSP_LCD_SetFont(&Font20); 
  
  // print the header and make it the color yellow
  BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
  BSP_LCD_DisplayStringAtLine(0, (uint8_t *)BOARDER);
  BSP_LCD_DisplayStringAtLine(1, (uint8_t *)header_message[0]);
  BSP_LCD_DisplayStringAtLine(2, (uint8_t *)header_message[1]);
  BSP_LCD_DisplayStringAtLine(3, (uint8_t *)BOARDER);
	
}

void lcd_data_display(void const *argument)
{

	//initalize the LCD display
	LCDdisplay();
	
	//print the thread status
	printf("LCD thread is up and running...\r\n");
	
	//initialise our message and priority
  message_t msg;
  uint8_t   priority;
	
	//begin while loop
	while(1)
	{
		
		//get the messages in the queue
		osStatus_t status = osMessageQueueGet(m_messages, 
												&msg, &priority, osWaitForever);
		
		//set  the status as a condition if its okay carry on
		if(status == osOK){
			
			//print the thermostat message to the LCD
			BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
			char thermoStr[25];
			sprintf (thermoStr, "Thermostat set temperature: %3.1f", msg.Thermostat);
			BSP_LCD_ClearStringLine(6);
			BSP_LCD_DisplayStringAtLine(6, (uint8_t *)thermoStr);
			
			//print the temperature message to the LCD
			char tempStr[25];
			sprintf (tempStr, "Temperature: %3.2f", msg.Temp);
			BSP_LCD_ClearStringLine(7);
			BSP_LCD_DisplayStringAtLine(7, (uint8_t *)tempStr);
			
			//print LDR ADC value to the LCD screen
			char ldrStr[20];
			sprintf(ldrStr, "LDR ADC value: %04d", msg.LDR);
			BSP_LCD_ClearStringLine(8);
			BSP_LCD_DisplayStringAtLine(8, (uint8_t *)ldrStr);
			
		}
		
		
	}

}
