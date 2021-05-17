#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <RBDdimmer.h>

#pragma once

//DEFINES
//MODULES ID
#define ID_MOD_MAIN							1
#define ID_MOD_WEATHER						11

//PINs
#define pinZERO_CROSSING					5
#define pinLIGHT0							0
#define pinLIGHT1							22
#define pinLIGHT2							23
#define pinLIGHT3							0

//ZONES
#define ID_ENTRANCE							0
#define ID_DRIVEWAY							1
#define ID_CARPORT							2
#define ID_FENCE							3

//DELAYS
#define DELAY_SENSOR_READ					10

struct Time {
	byte hour;
	byte minute;
};

struct Device {
	dimmerLamp lights[4];
	byte lightSensor = 20;
	byte turnOnLightLevel;
	byte standByIntensLevel;
	Time offTime;
	boolean nightTime=false;
};

void module_init();
void module();


