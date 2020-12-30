#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include "Adafruit_BME280.h"
#include <Servo.h>

#pragma once

//DEFINES
//SPI
#define SPI_SCK 							18
#define SPI_MISO 							19
#define SPI_MOSI							23

//BME280 CS PINS(SPI)
#define CS_BME280_CZERPNIA 					13
#define CS_BME280_WYRZUTNIA 				14
#define CS_BME280_NAWIEW	 				27
#define CS_BME280_WYWIEW	 				26

//BYPASS
#define PIN_BYPASS							25

//FAN
#define PWM_FREQUENCY 						1000 	// Hz
#define PWM_CHANNEL 						0		// this variable is used to select the channel number
#define PWM_RESOUTION 						8 		// resolution of the signal
#define PIN_FAN_PWM							33 		// GPIO to PWM fan input
#define PIN_FAN1_REVS						32		// FAN1 TACHO SIGNAL
#define PIN_FAN2_REVS						35		// FAN2 TACHO SIGNAL

struct SensorBME280 {
	float temperature = 0.0f;				// [stC]
	int pressure = 0;						// [hPa]
	int humidity = 0;						// [%]
	Adafruit_BME280 interface;
	unsigned int faultyReadings = 0;		// [units]
};

struct Device {
	SensorBME280 sensorsBME280[4];
	boolean normalON = false;
	boolean humidityAlert = false;
	boolean bypassOpen = false;
	int fanSpeed = 0;					// 0-100 [%]
	int fan1revs = 0;					// revs min-1
	int fan2revs = 0;					// revs min-1
	int hour[12];
};

void module_init();
void module();


