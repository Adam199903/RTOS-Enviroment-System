/*
 * rtos_objects.h
 *
 * this is the header file containing the rtos objects for the rtos mail queue
 * application
 *
 * this file takes the code used from lab 3 and lab 4 part 4 by alex shenfield
 * with changes implemented to suit our needs
 * 
 */

// define to prevent recursive inclusion
#ifndef __RTOS_OBJ_H
#define __RTOS_OBJ_H

// include the header file for basic data types and the cmsis-rtos2 api
#include <stdint.h>
#include "cmsis_os2.h"

// create the objects to use in our rtos applications to pass data

// my message queue
extern osMessageQueueId_t m_messages;

//flag for button press
extern osEventFlagsId_t button_flag;
extern osEventFlagsId_t button_flag2;
//extern osEventFlagsId_t IR_flag;

// message queue data structure
typedef struct 
{
	uint16_t  LDR;
	uint16_t  adctemp;
	float     Temp;
	float     Thermostat;
	int       PIR;
} 
message_t;


#endif // RTOS_OBJ_H
