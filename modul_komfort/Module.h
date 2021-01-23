#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#pragma once

//DEFINITIONS
//MODULES ID
#define ID_MOD_MAIN							1

//DALLAS 1-wire
#define DELAY_SENSORS_READ					10//20		// delay between sensors reading [s]
#define PIN_DS18B20							23

//CONST
#define	ZONE_QUANTITY						7			// quantity of zones

//Struktura strefy
struct Zone {
	DeviceAddress deviceAddress;			//Address DB18b20 1-wire
	int errorCount = 0;						// number of faulty reading
	int maxErrorCount = 0;						// number of faulty reading
	float isTemp=0;
	float reqTemp=0;
	int humidity=0;
	float directRead=0;
};

struct Device {
	Zone zone[ZONE_QUANTITY];
};

void module_init();
void module();

