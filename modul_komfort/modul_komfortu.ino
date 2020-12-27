#include "Arduino.h"
#include "Basic.h"
#include "EepromEvent.h"
#include "Communication.h"
#include "OTA.h"
#include "Module.h"
#include "Status.h"

Device device;
DateTime dateTime;

void setup()
{
//	Write values to eeprom
//	TMPwriteValueToEEPROM();
	DALLAS18b20ReadDeviceAdresses();

	FirstScan(&device);							// get values from eeprom after startup
	Serial_Setup(921600);						// serial port init with welcome text
	Pin_Setup();								// setup hardware pins
    WiFi_init();								// WiFi initialization
    OTA_init();									// Over the Air programming
    Module_init();								// Init sensors for main program
}

void loop()
{
	//OTA
	OTA_client();

	//Real Time
	DateTimeCalculation(&dateTime);

	//main program
	Module(&device);

	//Communication
	UDPsendStandardFrame(&device);
	UDPsendDiagnoseFrame(&device);
	UDPread(&device, &dateTime);

	//TEMP
	printStatus(device, dateTime);
}
