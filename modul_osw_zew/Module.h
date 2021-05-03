#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <RBDdimmer.h>

#pragma once

//DEFINES
//MODULES ID
#define ID_MOD_MAIN							1

//PINs
#define pinSENSOR							0
#define pinZERO_CROSSING					0
#define pinLIGHT0							0
#define pinLIGHT1							0
#define pinLIGHT2							0
#define pinLIGHT3							0

//ZONES
#define ID_ENTRANCE							0
#define ID_DRIVEWAY							1
#define ID_CARPORT							2
#define ID_FENCE							3

//DELAYS
#define DELAY_SENSOR_READ					10


struct Light {
	dimmerLamp dimmer;
	byte intens = 0;
};

struct Time {
	byte hour;
	byte minute;
};

struct Device {
	Light lights[4];
	byte lightSensor;
	byte turnOnLightLevel;
	byte standByIntensLevel;
	Time offTime;
};

void module_init();
void module();


