#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include "Adafruit_BME280.h"
#include <Servo.h>

#pragma once

//DEFINES
//MODULES ID
#define ID_MOD_MAIN							1
#define ID_MOD_COMFORT						10
#define ID_MOD_WEATHER						11
//ZONES ID
#define ID_ZONE_SALON						0
#define ID_ZONE_LAZDOL						1
#define ID_ZONE_PRALNIA						2
#define ID_ZONE_RODZICE						3
#define ID_ZONE_NATALIA						4
#define ID_ZONE_KAROLINA					5
#define ID_ZONE_LAZGORA						6

//SPI
#define PIN_SPI_SCK 						18 // BME280 PIN SCL
#define PIN_SPI_MISO 						19 // BME280 PIN SDO
#define PIN_SPI_MOSI						23 // BME280 PIN SDA

//BME280 CS PINS(SPI)
#define PIN_CS_BME280_CZERPNIA 				13 // BME280 PIN CSB
#define PIN_CS_BME280_WYRZUTNIA 			14 // BME280 PIN CSB
#define PIN_CS_BME280_NAWIEW	 			27 // BME280 PIN CSB
#define PIN_CS_BME280_WYWIEW	 			26 // BME280 PIN CSB
//BME280 IDs according to array declaration
#define ID_CZERPNIA							0
#define ID_WYRZUTNIA						1
#define ID_NAWIEW							2
#define ID_WYWIEW							3

//Fans
#define FAN_CZERPNIA						0
#define FAN_WYWIEW							1

//DS18b20
#define ID_WATER_INLET						0
#define ID_WATER_OUTLET						1
#define ID_AIR_INTAKE						2
#define ID_AIR_OUTLET						3

//BYPASS
#define SERVO_FREQUENCY 					50 		// Hz
#define SERVO_CHANNEL 						1		// this variable is used to select the channel number
#define SERVO_RESOUTION 					8 		// resolution of the signal
#define PIN_SERVO							25 		// GPIO to PWM fan input
//FAN
#define PWM_FREQUENCY 						1000 	// Hz
#define PWM_FAN_CZ_CHANNEL					2		// this variable is used to select the channel number
#define PWM_FAN_WY_CHANNEL					3		// this variable is used to select the channel number
#define PWM_RESOUTION 						8 		// resolution of the signal
#define PIN_FAN_CZ_PWM						33 		// GPIO to PWM fan input
#define PIN_FAN_WY_PWM						2 		// GPIO to PWM fan input
#define PIN_FAN_CZ_REVS						32		// FAN1 TACHO SIGNAL
#define PIN_FAN_WY_REVS						15		// FAN2 TACHO SIGNAL

//CIRCUIT PUMP PIN
#define PIN_CIRCUIT_PUMP					17
#define PIN_RELAY_RES						16

//I2C Servo Driver
#define PIN_I2C_SDA							21
#define PIN_I2C_SCL							22


struct SensorBME280 {
	float temperature = 0.0f;				// [stC]
	int pressure = 0;						// [hPa]
	float pressureHighPrec = 0.0f;
	int humidity = 0;						// [%]
	Adafruit_BME280 interface;
	unsigned int faultyReadings = 0;		// [units]
};

struct Fan {
	int speed = 0;							// 0-100%
	int rev = 0;							// revs min-1
	bool release=true;						// help var to get revs
};

struct Mode {
	bool trigger = false;				// trigger to turn on mode
	int triggerInt = 0;						// trigger to turn on mode
	bool turbo = false;					// turbo mode required (high fans revs)
	int delayTime = 0;						// delay time after trigger no more active
	int timeLeft = 0;
	unsigned long endMillis = 0;			// when mode triggered, here is store time when mode turns off
};

struct Matrix {
	bool salon = false;
	bool pralnia = false;
	bool lazDol = false;
	bool rodzice = false;
	bool Natalia = false;
	bool Karolina = false;
	bool lazGora = false;
};

struct FlapFresh {
	bool salon1 = false;
	bool salon2 = false;
	bool gabinet = false;
	bool warsztat = false;
	bool rodzice = false;
	bool natalia = false;
	bool karolina = false;
};

struct FlapUsed {
	bool kuchnia = false;
	bool lazDol1 = false;
	bool lazDol2 = false;
	bool pralnia = false;
	bool przedpokoj = false;
	bool garderoba = false;
	bool lazGora1 = false;
	bool lazGora2 = false;
};

struct ServoMotor {
	Servo interface;
	bool attached = false;
	int dutyCycle = 0;
	int lastDutyCycle = 0;
	unsigned long endMillis = 0;
};

struct Device {
	// byte 0
	bool humidityAlert = false;
	bool bypassOpen = false;
	bool circuitPump = false;
	bool reqPumpColdWater= false;
	bool reqPumpHotWater = false;
	bool reqAutoDiagnosis = false;
	bool defrostActive;

	// byte 1
	bool normalOn = false;
	bool activeCooling = false;
	bool activeHeating = false;
	bool reqLazDol = false;
	bool reqLazGora = false;
	bool reqKuchnia = false;

	//byte 2-17
	SensorBME280 sensorsBME280[4];			// sensory wew. reku

	//byte 18-21
	Fan fan[2];								// silniki

	//byte 22-25
	float heatExchanger[4];					// wymiennik ciep³a(ch³odnica za reku)

	//byte 30
	Mode normalMode;						// normal mode structure

	//byte 31-32
	Mode humidityAlertMode;					// humidity mode structure

	//byte 33-34
	Mode defrostMode;						// defrost mode structure

	//byte 35-58
	Matrix activeTempRegByHours[24];		// active cooling/heating according to hours
	Matrix zoneReqReg;

	//byte 59
	byte minTemp;							// min temp to trigger active cooling in zones. Priority is normal heating. Only when normal heating is to weak then trigger vent heating system

	//byte 60-83
	Matrix normalOnByHours[24];				// active cooling/heating according to hours

	//byte 89
	byte activeCoolingFanSpeed = 50;				// active cooling CZERPNIA fan speed

	FlapFresh flapFresh;
	FlapUsed flapUsed;

	ServoMotor byppass;
};

struct HandMode {
	bool enabled = false;
	byte fanSpeed = 0;
	bool byPassOpen = false;
};

struct TestMode {
	bool enabled = false;
};


struct Zone {
	int humidity = 0;
	float isTemp = 0.0f;
	float reqTemp = 0.0f;
};

struct AirPollution {
	int pm25=0;
	int pm10=0;
};

void module_init();
void module();


