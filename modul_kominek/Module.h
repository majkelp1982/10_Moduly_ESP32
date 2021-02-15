#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#pragma once

//DEFINES
//MODULES ID
#define ID_MOD_MAIN							1
#define ID_MOD_HEATING						14

//DALLAS ID
#define ID_INLET							0
#define ID_OUTLET							1
#define ID_CHIMNEY							2

//DALLAS 1-wire
#define DELAY_SENSORS_READ					10			// delay between sensors reading [s]
#define PIN_DS18B20							21

struct Thermometer {						// Thermometer
	DeviceAddress deviceAddress;
	float isTemp=0;							// measured temperature
	int errorCount = 0;						// number of faulty reading
	int maxErrorCount = 0;
};

struct HeatingDevice {
	bool pumpInHouse = false;
	unsigned long paramsLastTimeRead = 0;
};

struct Device {
	int mode = 1;
	bool alarm = false;
	bool warning = false;
	bool pump = false;
	bool fireAlarm = false;
	Thermometer thermo[3];
	int setTemp;
	int throtlle = 0;
};

void module_init();
void module();


