#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include "Adafruit_BME280.h"
#include <SDS011.h>

#pragma once

//DEFINES
//MODULES ID
#define ID_MOD_MAIN							1

//SPI
#define SPI_SCK 							18 // BME280 PIN SCL
#define SPI_MISO 							19 // BME280 PIN SDO
#define SPI_MOSI							23 // BME280 PIN SDA

//BME280 CS PIN(SPI)
#define CS_BME280							13 // BME280 PIN CSB

#define SERIAL2_RX							16 // Serial2 RX PIN
#define SERIAL2_TX							17 // Serial2 TX PIN

struct SensorBME280 {
	float temperature = 0.0f;				// [stC]
	int pressure = 0;						// [hPa]
	int humidity = 0;						// [%]
	Adafruit_BME280 interface;
	unsigned int faultyReadings = 0;		// [units]
};

struct SensorSDS011 {
	unsigned long modeTimeLeft;
	int pm25 = 0;
	int pm10 = 0;
	SDS011 interface;
	unsigned int faultyReadings = 0;		// [units]
	int sleepTime = 600;					// time in [s] when sensor stay sleep
	int standUpTime = 10;					// time in [s] when sensor after wake up need to stable parameters
};

struct Device {
	SensorBME280 sensorBME280;
	SensorSDS011 sensorSDS011;
};

void module_init();
void module();


