#include "Arduino.h"
#include "Basic.h"
#include "EepromEvent.h"
#include "Communication.h"
#include "OTA.h"
#include "Module.h"
#include "Status.h"

DateTime dateTime;
Device device;

void setup()
{
	Serial_Setup(921600);						// serial port init with welcome text
	//	Write values to eeprom
	//  TMPwriteValueToEEPROM();

	FirstScan(&device);							// get values from eeprom after startup
	Serial.println("First scan done");
	Pin_Setup();								// setup hardware pins
	Serial.println("Pin setup done");
	WiFi_init();								// WiFi initialization
	Serial.println("WiFi init done");
	OTA_init();									// Over the Air programming
	Serial.println("OTA done");
	Module_init();								// Init sensors for main program
	Serial.println("Module init done ");
	Valves_init(&device);
	Serial.println("Valves init done");
}

void loop()
{
	//OTA
	OTA_client();

	//Real Time
	DateTimeCalculation(&dateTime);

	//main program
	Module(&device,dateTime);

	//Communication
	UDPsendStandardFrame(&device);
	UDPsendDiagnoseFrame(&device);
	UDPread(&device, &dateTime);

	//TEMP
	printStatus(device, dateTime);
}
