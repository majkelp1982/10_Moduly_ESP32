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
//ZONES ID
#define ID_ZONE_SALON						0
#define ID_ZONE_LAZDOL						1
#define ID_ZONE_PRALNIA						2
#define ID_ZONE_RODZICE						3
#define ID_ZONE_NATALIA						4
#define ID_ZONE_KAROLINA					5
#define ID_ZONE_LAZGORA						6

//SPI
#define SPI_SCK 							18 // BME280 PIN SCL
#define SPI_MISO 							19 // BME280 PIN SDO
#define SPI_MOSI							23 // BME280 PIN

//BME280 CS PINS(SPI)
#define CS_BME280_CZERPNIA 					13 // BME280 PIN CSB
#define CS_BME280_WYRZUTNIA 				14 // BME280 PIN CSB
#define CS_BME280_NAWIEW	 				27 // BME280 PIN CSB
#define CS_BME280_WYWIEW	 				26 // BME280 PIN CSB
//BME280 IDs according to array declaration
#define ID_CZERPNIA							0
#define ID_WYRZUTNIA						1
#define ID_NAWIEW							2
#define ID_WYWIEW							3

//BYPASS
#define SERVO_FREQUENCY 					50 		// Hz
#define SERVO_CHANNEL 						2		// this variable is used to select the channel number
#define SERVO_RESOUTION 					8 		// resolution of the signal
#define SERVO_PIN							25 		// GPIO to PWM fan input
//FAN
#define PWM_FREQUENCY 						1000 	// Hz
#define PWM_CHANNEL 						1		// this variable is used to select the channel number
#define PWM_RESOUTION 						8 		// resolution of the signal
#define PIN_FAN_PWM							33 		// GPIO to PWM fan input
#define PIN_FAN1_REVS						32		// FAN1 TACHO SIGNAL
#define PIN_FAN2_REVS						12		// FAN2 TACHO SIGNAL

//HUMIDITY ALLERT
#define HUMIDITY_TO_HIGH					80
#define HUMIDITY_ALERT_PROCESS_TIME			10

struct SensorBME280 {
	float temperature = 0.0f;				// [stC]
	int pressure = 0;						// [hPa]
	int humidity = 0;						// [%]
	Adafruit_BME280 interface;
	unsigned int faultyReadings = 0;		// [units]
};

struct Mode {
	boolean req = false;				// req to set defrost mode in case recuperator is frozen
	int timeLeft = 0;					// time left to finish defrost process [min]
	int trigger = 0;					// difference between inlet and out pressure to confirm recu stuck because of ice [hPa]
	unsigned long endMillis = 0;
};

struct Device {
	SensorBME280 sensorsBME280[4];
	boolean normalON = false;			// normal mode
	Mode humidityAlert;					// humidity mode structure
	boolean bypassOpen = false;			// bypass open in case defrost or cooling in summer night
	Mode defrost;						// defrost mode structure
	int fanSpeed = 0;					// 0-100 [%]
	int fan1revs = 0;					// revs min-1
	int fan2revs = 0;					// revs min-1
	int hour[12];
	int efficency = 0;
	int efficencyMAX = 0;
};

struct Zone {
	int humidity = 0;
	float isTemp = 0.0f;
	float reqTemp = 0.0f;
};

void module_init();
void module();


