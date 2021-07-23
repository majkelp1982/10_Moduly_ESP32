#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <DHT.h>
#include "Adafruit_BME280.h"

#pragma once

//DEFINITIONS
//MODULES ID
#define ID_MOD_MAIN							1

//ZONES NAMES
#define ID_SALON							0
#define ID_PRALNIA							1
#define ID_LAZ_DOL							2
#define ID_RODZICE							3
#define ID_NATALIA							4
#define ID_KAROLINA							5
#define ID_LAZ_GORA							6
#define ID_GABINET							7

//SPI
#define PIN_SPI_SCK 						18 // BME280 PIN SCL
#define PIN_SPI_MISO 						19 // BME280 PIN SDO
#define PIN_SPI_MOSI						23 // BME280 PIN SDA

//BME280 CS PINS(SPI)
#define PIN_CS_BME280_NATALIA 				13 // BME280 PIN CSB

//DALLAS 1-wire
#define DELAY_SENSORS_READ					20			// delay between sensors reading [s]
#define PIN_DS18B20							21

//DHT HUMIDITY SENSORS
#define DHTTYPE 							11  		//DHT 11
//DHT PINS
#define PIN_DHT_SALON						14
#define PIN_DHT_PRALNIA						34
#define PIN_DHT_LAZ_DOL						0
#define PIN_DHT_RODZICE						35
#define PIN_DHT_NATALIA						33
#define PIN_DHT_KAROLINA					25
#define PIN_DHT_LAZ_GORA					26

//CONST
#define	ZONE_QUANTITY						7			// quantity of zones

//Struktura strefy
struct Zone {
	DeviceAddress deviceAddress;			//Address DB18b20 1-wire
	int dallasErrorCount = 0;				// number of faulty reading
	int dallasMaxErrorCount = 0;			// number of faulty reading
	int dhtErrorCount = 0;					// number of faulty reading
	int dhtMaxErrorCount = 0;				// number of faulty reading
	float isTemp=0.0f;
	float reqTemp=0;
	int humidity=0;
	float dhtTemp = 0.0f;
};

struct SensorBME280 {
	float temperature = 0.0f;				// [stC]
	int pressure = 0;						// [hPa]
	float pressureHighPrec = 0.0f;
	int humidity = 0;						// [%]
	Adafruit_BME280 interface;
	unsigned int faultyReadings = 0;		// [units]
};


struct Device {
	Zone zone[ZONE_QUANTITY];
	SensorBME280 sensorsBME280[8];
	DHT dhtSensor[7];
};

void module_init();
void module();

