#include "Arduino.h"
#include "Basic.h"
#include "EepromEvent.h"
#include "Communication.h"
#include "FaultDetection.h"
#include "Ventilation.h"
#include "OTA.h"
#include "Status.h"

Device device;
DateTime dateTime;
Humidity humidity;
Temperature temperature;
AirHeatingActive airHeatingActive;

void Outputs();

void setup()
{
	FirstScan(&device);							// get values from eeprom after startup
	Serial_Setup(921600);						// serial port init with welcome text
	Pin_Setup();								// setup hardware pins
    WiFi_init();								// WiFi initialization
    OTA_init();
}

void loop()
{
	//OTA
	OTA_client();

	//Real Time
	DateTimeCalculation(&dateTime);

	//Communication
	UDPsendStandardFrame(&device);
	UDPsendDiagnoseFrame(&device);
	UDPread(&device, &humidity, &temperature, &dateTime);

	//Ventilation
	Ventilation(&device, humidity, dateTime);

	//FaultDetection
	FaultsDetection();

	Outputs();

	//TEMP
	printStatus(device, humidity, temperature, dateTime);
}

void Outputs() {
	// Fan
	device.fan = false;
	device.fan = (device.normalOn) || (device.humidityAlert);

	//Outputs
	digitalWrite(FAN,!device.fan);
	digitalWrite(PUMP,!device.pump);
}
