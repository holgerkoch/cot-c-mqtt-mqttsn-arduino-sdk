/**
 * @description Simple Demo for CoT Library using MQTT
 * @copyright (C) 2017-2018 Deutsche Telekom AG- all rights reserved. 
 * @licence MIT licence
 */
/** 
   This project demonstrates how simple it is to build a DTCoT application

   This example also shows that DTCoT does not depend on device type,
   network and protocol libraies, and can be easily extended by adding more
   devices and underlaying protocols (as soon as the cloud starts supporting them).

   Application description:
   In this demo a simulated temperature measurement is sent to the cloud.
   In this demo multiple alarms are sent to the cloud.
   In this demo multiple operations are registered to receive from the cloud.

   This sketch has the following library dependencies:
		- <Adafruit MQTT Library> v0.20.1
		- <ESP8266WiFi> v1.0
        - <Time> v1.5

*/


#include "dt-arduino-iot-agent.h"
#include "cot_sdk/DTCotSdkMqtt.h"
#include "cot_sdk/DTCoTDevice.h"

#include "devices/DTCoTDeviceHUZZAH_WIFI.h"

#include "DTCotLed.h"

/************************* WiFi Access Point *********************************/

//#define SECRET_WIFI_SSID       "...your local Wifi network's SSID..."
//#define SECRET_WIFI_PASSWORD   "...your local Wifi network's password..."
#define SECRET_WIFI_SSID		"IOT-LAB-BRE"
#define SECRET_WIFI_PASSWORD	"ds1&tnova%49265++"

/*************************** Sketch Code ************************************/

using namespace DTCoT;

/* Application-specific settings */
float tempMin = 20;
float tempMax = 30;
float tempInc = 0.1;
float temp = tempMin;
bool restartFlag = false;
unsigned long startTS;
unsigned long loopCount = 0;
unsigned long loopInterval = 60000; //ms
unsigned long loopStart;
unsigned long lastLoopStart = 0;


//temperatue sensor fake
float readTemp() {
    float t = temp;
    temp += tempInc;
    if (temp > tempMax) {
        temp = tempMin;
    }
    return t;
}

//info led
DTCotLed led(LED_BUILTIN, false);

//operation function to switch led on
OperationStatus operationLedOn(std::vector<String>&) {
	DT_DEBUG_PRINTLN("");
    DT_DEBUG_PRINTLN("#####################################");
    DT_DEBUG_PRINTLN("##  Operation LED on");
    DT_DEBUG_PRINTLN("#####################################");
    led.turnOn(true);
    return OperationStatus::SUCCESSFUL;
}

//operation function to switch led off
OperationStatus operationLedOff(std::vector<String>&) {
	DT_DEBUG_PRINTLN("");
    DT_DEBUG_PRINTLN("#####################################");
    DT_DEBUG_PRINTLN("##  Operation LED off");
    DT_DEBUG_PRINTLN("#####################################");
    led.turnOn(false);
    return OperationStatus::SUCCESSFUL;
}

//operation function to toggle led
OperationStatus operationLedToggle(std::vector<String>&) {
	DT_DEBUG_PRINTLN("");
    DT_DEBUG_PRINTLN("#####################################");
    DT_DEBUG_PRINTLN("##  Operation LED toggle");
    DT_DEBUG_PRINTLN("#####################################");
    led.toggle();
    return OperationStatus::SUCCESSFUL;
}

//operation function to let led blink
OperationStatus operationLedBlinkOn(std::vector<String>&) {
	DT_DEBUG_PRINTLN("");
    DT_DEBUG_PRINTLN("#####################################");
    DT_DEBUG_PRINTLN("##  Operation LED blink");
    DT_DEBUG_PRINTLN("#####################################");
    led.blink(true);
    return OperationStatus::SUCCESSFUL;
}

//operation function to switch led blink off
OperationStatus operationLedBlinkOff(std::vector<String>&) {
	DT_DEBUG_PRINTLN("");
    DT_DEBUG_PRINTLN("#####################################");
    DT_DEBUG_PRINTLN("##  Operation LED blink off");
    DT_DEBUG_PRINTLN("#####################################");
    led.blink(false);
    return OperationStatus::SUCCESSFUL;
}

//operation function to set a flag to restart the device
OperationStatus operationRestart(std::vector<String>& messageParts) {
	DT_DEBUG_PRINTLN("");
    DT_DEBUG_PRINTLN("#####################################");
    DT_DEBUG_PRINTLN("##  Operation RESTART<%s>", messageParts[2].c_str());
    DT_DEBUG_PRINTLN("#####################################");
    restartFlag = true;
    return OperationStatus::SUCCESSFUL;
}

//function to set check the restart flag and execute restart if set.
void restartDeviceIfFlaged(void) {
    if (restartFlag) {
    	DT_DEBUG_PRINTLN("");
    	DT_DEBUG_PRINTLN("#####################################");
    	DT_DEBUG_PRINTLN("##  Restarting device...");
    	DT_DEBUG_PRINTLN("#####################################");
        //signal restart will be executed immediately
        led.toggle(10, 100);
        // processor software reset
        ESP.restart();
        while(true);
    }
}

//operation function to log the desire to update the firmware
OperationStatus operationFirmware(std::vector<String>&) {
	DT_DEBUG_PRINTLN("");
	DT_DEBUG_PRINTLN("#####################################");
	DT_DEBUG_PRINTLN("##  Operation FIRMWARE");
	DT_DEBUG_PRINTLN("#####################################");
    return OperationStatus::SUCCESSFUL;
}

//My CoT device
CoTDevice cotDevice;

void setup() {
	// 'Serial' is USB, so... be carefull
  	delay(100);
  	Serial.begin(115200);
    delay(5000);
    DT_DEBUG_PRINTLN("\nSetup...");
    DT_DEBUG_PRINTLN("DT_COT_ARDUINO_LIBRARY_VERSION <%s>", DT_COT_ARDUINO_LIBRARY_VERSION);

	/* Setup the Cloud access and the device to communicate to cloud. */
    CoTConfigDeviceHuzzahWifi* boardConfig = new CoTConfigDeviceHuzzahWifi(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
	CoTDeviceHuzzahWifi* board = new CoTDeviceHuzzahWifi(*boardConfig);
	CotSdkMqtt* cotSDK = new CotSdkMqtt(*board);

//    boardConfig->setDevicePrefix("TU96.HEHE100.");
//    boardConfig->setDeviceId("HUZ.1018154");
//    cotSDK->setCotCredentials("HUZ.1018154", "M7AIfZdP!U", "495301");

    //initialize the device
    while(!cotDevice.setup(cotSDK));

    //register the operation functions
    cotDevice.registerOperation(operationLedOn, DTCOTOPERATIONID_C8Y_COMMAND, "led on");
    cotDevice.registerOperation(operationLedOff, DTCOTOPERATIONID_C8Y_COMMAND, "led off");
    cotDevice.registerOperation(operationLedToggle, DTCOTOPERATIONID_C8Y_COMMAND, "led toggle");
    cotDevice.registerOperation(operationLedBlinkOn, DTCOTOPERATIONID_C8Y_COMMAND, "led blink");
    cotDevice.registerOperation(operationLedBlinkOff, DTCOTOPERATIONID_C8Y_COMMAND, "led blink off");
    cotDevice.registerOperation(operationRestart, DTCOTOPERATIONID_C8Y_COMMAND, "restart");
    cotDevice.registerOperation(operationRestart, DTCOTOPERATIONID_C8Y_RESTART, "");
    cotDevice.registerOperation(operationFirmware, DTCOTOPERATIONID_C8Y_FIRMWARE, "");

    //signal setup is finished
    led.toggle(4, 100);

    //timestamp of device start
    startTS = now();

    DT_DEBUG_PRINTLN("Setup... done.");
}

void loop() {
    loopStart = millis();
    //check for incoming operations
    cotDevice.executeOperations();
    //trigger led status, because no interrupt is in use for!
    led.trigger();
    //check the restart flag
    restartDeviceIfFlaged();
    if (!lastLoopStart || loopStart > (lastLoopStart + loopInterval)) {
        lastLoopStart = loopStart;
        loopCount++;
    	cotDevice.sendAlarm("alarmCritical", "battery_problem", AlarmSeverity::CRITICAL, AlarmStatus::ACTIVE);
    	long ts = now();
    	cotDevice.sendMeasurement("ty", "mo", "t", "C", readTemp(), ts);
    	cotDevice.sendMeasurement("pi", "ping", "n", "P", float(loopCount), ts);
    }
}
