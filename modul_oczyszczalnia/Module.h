#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SH1106.h>
#include <UltrasonicSensor.h>

#pragma once

//DELAY
#define DELAY_SENSOR_READ			10
#define DELAY_LIMIT_READ			10					// IMPORTANT!! After Limit sensor turning on water pump. 10s needed for pump to get rid of large amount of water

//DEFINES
//MODULES ID
#define ID_MOD_MAIN					1

//PINS
//PUMPS
#define pinAIR_PUMP					13
#define pinWATER_PUMP				12

//HC-SR04 Ultrasonic Sensor
#define pinECHO						23
#define pinTRIG						22

//LIMIT SENSOR
#define pinLIMIT					21

struct Device {
	bool airPump = true;
	bool waterPump = false;
	bool limitSensor = false;
	int isWaterLevelZeroRef;
	int maxWaterLevel;
	int minWaterLevel;
	int zeroReference;
	int isWaterLevel;
	byte airInterval;
	unsigned long lastStateChange = 0;
};

void module_init();
void module();


