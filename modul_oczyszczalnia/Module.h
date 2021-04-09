#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SH1106.h>

#pragma once

//DELAY
#define DELAY_SENSOR_READ			10

//DEFINES
//MODULES ID
#define ID_MOD_MAIN					1

//PINS
//PUMPS
#define pinAIR_PUMP					0
#define pinWATER_PUMP				0

//HC-SR04 Ultrasonic Sensor
#define pinTRIG						0
#define pinECHO						0

struct Device {
	bool airPump = false;
	bool waterPump = false;
	byte isWaterLevel;
	byte maxWaterLevel;
	byte minWaterLevel;
	byte airInterval;
	byte zeroReference;
	unsigned long lastStateChange = 0;
};

void module_init();
void module();


