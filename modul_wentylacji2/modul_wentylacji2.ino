#include "Arduino.h"
#include "Basic.h"
#include "Communication.h"
#include "OTA.h"
#include "Module.h"
#include "Status.h"

DateTime dateTime;
Device device;

void setup()
{
//	FirstScan(&device);							// get values from eeprom after startup
	Serial_Setup(921600);						// serial port init with welcome text
	Pin_Setup();								// setup hardware pins
    WiFi_init();								// WiFi initialization
    OTA_init();
	Module_init(&device);
}

// The loop function is called in an endless loop
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
