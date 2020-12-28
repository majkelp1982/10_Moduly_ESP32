#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include "Adafruit_BME280.h"

#pragma once

//PINS
//BME280
#define CS_BME280_CZERPNIA 					13
#define CS_BME280_WYRZUTNIA 				14
#define CS_BME280_NAWIEW	 				27
#define CS_BME280_WYWIEW	 				26

//SPI
#define SPI_SCK 							18
#define SPI_MISO 							19
#define SPI_MOSI							23

struct SensorBME280 {
	float temperature = 0.0f;
	int pressure = 0;
	int humidity = 0;
	Adafruit_BME280 interface;
	unsigned int faultyReadings = 0;
};

struct Device {
	SensorBME280 sensorsBME280[4];
	bool fan = false;
	boolean normalON = false;
	boolean humidityAlert = false;
	int hour[12];
};

void module_init();
void module();


