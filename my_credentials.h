/*
 * my_credentials.h
 *
 * this is the header file containing my wifi credentials and mqtt account
 * details (if needed)
 */

// define to prevent recursive inclusion
#ifndef __CREDS_H
#define __CREDS_H

// wifi credentials
#define SSID           ""
#define PASSWORD       ""
#define SECURITY_TYPE   ARM_WIFI_SECURITY_WPA2

//
// NOTE - make sure this clientID is unique. i recommend using your student
// id number
//

// mqtt credentials
#define mqtt_clientid  ""
#define mqtt_username  ""
#define mqtt_password  ""

#endif
