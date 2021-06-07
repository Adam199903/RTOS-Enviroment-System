/*
 * run_mqtt.c
 *
 * this file contains the logic for connecting to an mqtt broker over tls
 * and sending it some messages
 *
 * this code is based on lab 4 part 3 / 4 provided by alex shenfield
 * plus our own origional code 
 * 
 */

// include the c string handling functions
#include <string.h>

// include the paho mqtt client
#include "MQTTClient.h"

// include our rtos objects (including the message object)
#include "rtos_objects.h"

//gpio support
#include "pinmappings.h"
#include "gpio.h"

// include the root ca certificate (and client certificate and key - but we're
// not using those ...)
#include "certificates.h"

// include my mqtt credentials
#include "my_credentials.h"

// define the mqtt server
#define SERVER_NAME 	"test.mosquitto.org"
#define SERVER_PORT 	1883

// MQTT CALLBACK

// set up our message received callback (just dump the message to the screen)
void message_received(MessageData* data)
{
  printf("message arrived on topic %.*s: %.*s\n",
         data->topicName->lenstring.len,
         data->topicName->lenstring.data,
         data->message->payloadlen,
         (char *)data->message->payload);
}

// light contol LED light bulb
gpio_pin_t led = {PB_14, GPIOB, GPIO_PIN_14};

// security ON LED
gpio_pin_t led2 = {PB_15, GPIOB, GPIO_PIN_15};

void security_cb(MessageData* data)
{
  // get the message from the message data object
  char message_text2[128];
  sprintf(message_text2, "%.*s", data->message->payloadlen, (char *)data->message->payload);

  // parse the message payload
  if(strcmp(message_text2, "ON") == 0)
  {
    printf("turning on security\n");
    write_gpio(led2, HIGH);
  }
  else if(strcmp(message_text2, "OFF") == 0)
  {
    printf("turning off security\n");
    write_gpio(led2, LOW);
  }
}

// set up light switch topic callback
void light_switch_cb(MessageData* data)
{
  // get the message from the message data object
  char message_text[128];
  sprintf(message_text, "%.*s", data->message->payloadlen, (char *)data->message->payload);
	
  // parse the message payload
  if(strcmp(message_text, "ON") == 0)
  {
    printf("turning on led\n");
    write_gpio(led, HIGH);
  }
  else if(strcmp(message_text, "OFF") == 0)
  {
    printf("turning off led\n");
    write_gpio(led, LOW);
  }
}
// MQTT THREAD

// set up the mqtt connection as a thread
void mqtt_run_task(void *argument)
{
	
	

  // initialise structures for the mqtt client and for the mqtt network
  // connection
  MQTTClient client;
  Network network;
	
	// initialise the gpio for the light switch light bulb
  init_gpio(led, OUTPUT);
  write_gpio(led, LOW);
	
	// initialise the gpio for the security light bulb
  init_gpio(led2, OUTPUT);
  write_gpio(led2, LOW);

  // establish a secured network connection to the broker

  // initialise the network connection structure
  NetworkInit(&network);

  // try to connect to the mqtt broker using tls and - if it fails - print out
  // the reason why
  int rc = 0;
  if((rc = NetworkConnect(&network, SERVER_NAME, SERVER_PORT)) != 0)
  {
    printf("network connection failed - "
           "return code from network connect is %d\n", rc);
  }

  // connect the client to the broker

  // initialise the mqtt client structure with buffers for the transmit and
  // receive data
  unsigned char sendbuf[128];
  unsigned char	readbuf[128];
  MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf),
                 readbuf, sizeof(readbuf));

  // initialise the mqtt connection data structure
  MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
  connectData.MQTTVersion = 4;
  connectData.clientID.cstring = (char*)mqtt_clientid;
  connectData.username.cstring = (char*)mqtt_username;
  connectData.password.cstring = (char*)mqtt_password;

  // start mqtt client as task (what this does is to make sure that the mqtt
  // processing functionality is protected by a mutex so it can only run
  // sequentially - i assume this means we can have subscriptions and
  // publishing in multiple threads ... but we'll have to pass the client into
  // those threads when they are started)
  if((rc = MQTTStartTask(&client)) != 1)
  {
    printf("return code from start tasks is %d\n", rc);
  }

  // connect ...
  if((rc = MQTTConnect(&client, &connectData)) != 0)
  {
    printf("return code from MQTT connect is %d\n", rc);
  }
  else
  {
    printf("MQTT Connected\n");
  }

  // manage mqtt topic subscriptions

  // subscribe to the everything in the root ('#') of my test topic (we will
  // publish messages to a subtopic of this root so this subscription means we
  // can see what we're sending). we also need to register the callback that
  // will be triggered everytime a message on that topic is received (here this
  // is "message_received" from earlier) and set the QoS level (here we are
  // using level 2 - but you need to check what your broker supports ...)
  char topic[] = "Adam/iot-mqtt/#";
  if((rc = MQTTSubscribe(&client, topic, QOS2, message_received)) != 0)
  {
    printf("return code from MQTT subscribe is %d\n", rc);
  }
	
	// subscribe to a light switch topic
  char light_switch_topic[] = "Adam/iot-mqtt/Enviroment/light-switch";
  if((rc = MQTTSubscribe(&client, light_switch_topic, QOS2, light_switch_cb)) != 0)
  {
    printf("return code from MQTT subscribe is %d\n", rc);
  }
	
	// subscribe to a security topic
  char security_topic[] = "Adam/iot-mqtt/Security system/System stastus";
  if((rc = MQTTSubscribe(&client, security_topic, QOS2, security_cb)) != 0)
  {
    printf("return code from MQTT subscribe is %d\n", rc);
  }
	

  // publish messages

  // initialise our message object and the message priority
  message_t msg;
  uint8_t   priority;

  // run our main mqtt publish loop
  while(1)
  {
    // get a message from the message queue
    osStatus_t status = osMessageQueueGet(m_messages, &msg, &priority,
                                                        osWaitForever);
		
		// check for lights button press (returning immediately)
    uint32_t LightSwitchFlag = osEventFlagsWait(button_flag, 0x01, osFlagsWaitAny, 0);
		
		// check for security button press (returning immediately)
    uint32_t securityFlag = osEventFlagsWait(button_flag2, 0x01, osFlagsWaitAny, 0);
		
		
		//-------------------------------bounce back security----------------------------------	

			if(securityFlag == 0x01){
				
			// initialise a character array for the message payload
      char payload[64];

      // create the basic message and set up the publishing settings
      MQTTMessage message;
      message.qos = QOS1;
      message.retained = 0;
      message.payload = payload;
				
			// send the light switch message
      // write the data into the payload and send it
      memset(payload, 0, sizeof(payload));
      if(read_gpio(led2))
      {
        sprintf(payload, "OFF");
        message.payloadlen = strlen(payload);
      }
      else
      {
        sprintf(payload, "ON");
        message.payloadlen = strlen(payload);
					
      }

      // publish the message (printing the error if something went wrong)
      if((rc = MQTTPublish(&client, security_topic, &message)) != 0)
      {
        printf("return code from MQTT publish is %d\n", rc);
      }
		}
//--------------------------------Movment detector---------------------------------
		
		if(read_gpio(led2)){
			
			// initialise a character array for the message payload
      char payload[64];

      // create the basic message and set up the publishing settings
      MQTTMessage message;
      message.qos = QOS1;
      message.retained = 0;
      message.payload = payload;			
					
			memset(payload, 0, sizeof(payload));
			if(msg.PIR == HIGH){
				
				sprintf(payload, "Intruder detected");
				message.payloadlen = strlen(payload);
				
			}
			else{
				
				sprintf(payload, "No movement detected");
				message.payloadlen = strlen(payload);
				
			}
				
			
			
			// publish the message (printing the error if something went wrong)
			char Intruder_topic[] = "Adam/iot-mqtt/Security system/intruder";
			if((rc = MQTTPublish(&client, Intruder_topic, &message)) != 0)
					{
						printf("return code from MQTT publish is %d\n", rc);
					}	
		}
			
		
//-------------------------------bounce back light switch----------------------------------	

			if(LightSwitchFlag == 0x01){
				
			// initialise a character array for the message payload
      char payload[64];

      // create the basic message and set up the publishing settings
      MQTTMessage message;
      message.qos = QOS1;
      message.retained = 0;
      message.payload = payload;
				
			// send the light switch message
      // write the data into the payload and send it
      memset(payload, 0, sizeof(payload));
      if(read_gpio(led))
      {
        sprintf(payload, "OFF");
        message.payloadlen = strlen(payload);
      }
      else
      {
        sprintf(payload, "ON");
        message.payloadlen = strlen(payload);
      }

      // publish the message (printing the error if something went wrong)
      if((rc = MQTTPublish(&client, light_switch_topic, &message)) != 0)
      {
        printf("return code from MQTT publish is %d\n", rc);
      }
		}
//-------------------------enviroment messages------------------------------------
		
    // check the message status
    if(status == osOK)
    {
      // initialise a character array for the message payload
      char payload[64];

      // create the basic message and set up the publishing settings
      MQTTMessage message;
      message.qos = QOS1;
      message.retained = 0;
      message.payload = payload;

//-------------------------------LDR message to mqtt-------------------------------
      // send the LDR sensor value
      // write the data into the payload and send it
      memset(payload, 0, sizeof(payload));
      sprintf(payload, "%04d", msg.LDR);
      message.payloadlen = strlen(payload);

      // publish the message (printing the error if something went wrong)
      char LDR_topic[] = "Adam/iot-mqtt/Enviroment/LDR";
      if((rc = MQTTPublish(&client, LDR_topic, &message)) != 0)
      {
        printf("return code from MQTT publish is %d\n", rc);
      }

//--------------------------------Temperature message sent to mqtt------------------
      // send the temp sensor value
      // write the data into the payload and send it
      memset(payload, 0, sizeof(payload));
      sprintf(payload, "%04d", msg.adctemp);
      message.payloadlen = strlen(payload);

      // publish the message (printing the error if something went wrong)
      char tempadc_topic[] = "Adam/iot-mqtt/Enviroment/Temperatureadc";
      if((rc = MQTTPublish(&client, tempadc_topic, &message)) != 0)
      {
        printf("return code from MQTT publish is %d\n", rc);
      }
//--------------------------------Temperature message sent to mqtt------------------
      // send the temp sensor value
      // write the data into the payload and send it
      memset(payload, 0, sizeof(payload));
      sprintf(payload, "%3.2f", msg.Temp);
      message.payloadlen = strlen(payload);

      // publish the message (printing the error if something went wrong)
      char temp_topic[] = "Adam/iot-mqtt/Enviroment/Temperature";
      if((rc = MQTTPublish(&client, temp_topic, &message)) != 0)
      {
        printf("return code from MQTT publish is %d\n", rc);
      }
//---------------------------------Thermostat value sent to mqtt----------------------		
			 // send the temp sensor value
      // write the data into the payload and send it
      memset(payload, 0, sizeof(payload));
      sprintf(payload, "%3.1f", msg.Thermostat);
      message.payloadlen = strlen(payload);

      // publish the message (printing the error if something went wrong)
      char thermostat_topic[] = "Adam/iot-mqtt/Enviroment/Thermostat";
      if((rc = MQTTPublish(&client, thermostat_topic, &message)) != 0)
      {
        printf("return code from MQTT publish is %d\n", rc);
      }
		} 

 }

  // we don't ever get here - but could keep an error counter for our publish
  // loops and exit if we have too many errors

  //// disconnect from the network
  //NetworkDisconnect(&network);
  //printf("MQTT disconnected\n");
}
