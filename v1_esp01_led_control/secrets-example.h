#ifndef _ARDUINO_SECRETS_H_
#define _ARDUINO_SECRETS_H_

#define DEBUG 1

//Wifi parameters

#define SECRET_SSID "your-network-name"
#define SECRET_PASS "your-password"
#define SECRET_SSID2 "backup-network-name"
#define SECRET_PASS2 "backup-password"

//MQTT parameters

#define BROKER_Server "server-address"  // MQTT broker IP or hostname
#define BROKER_User "emqx"
#define BROKER_Password "public"
#define BROKER_Port 1883                // Default MQTT port
#define BROKER_Topic "your-topic"      // MQTT topic for controlling LED

// HW parameters
#endif
