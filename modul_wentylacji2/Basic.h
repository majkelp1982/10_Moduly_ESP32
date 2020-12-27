#include "Arduino.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BME280.h"

// Definitions for modul wentylacji

#pragma once
//Firmware version
#define Firm_VERSION 						20122015	// rok miesiac dzien godzina
#define DEVICE_TYP							13			// modul wentylacji
#define DEVICE_NO							0			// numer modulu wentylacji

//NETWORK
#define WIFI_SSID 							"Majkel"
#define WIFI_PSWD 							"12345678"
#define LOCAL_PORT							6000

// UDP
#define DELAY_BETWEEN_UDP_STANDARD			10		// delay between UDP send [s]
#define DELAY_BETWEEN_UDP_DIAGNOSE			30		// delay between UDP send [s]
#define FRAME_STANDARD_SIZE					30
#define FRAME_DIAGNOSE_SIZE					6

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

//Structure
struct Diagnose {
	byte ip[4];
	byte wifiConnectionInterrupt = 0;
	byte wifiConnected = false;
};

struct Device {
	Diagnose diagnose;
	SensorBME280 sensorsBME280[4];
};

struct DateTime {
	int year=0;
	int month=0;
	int day=0;
	int weekDay=0;
	int hour=12;
	int minute=0;
	int second=0;
};

void Pin_Setup();
void Serial_Setup(int baud);
bool Sleep(unsigned long *lastMillis, int seconds);

void DateTimeCalculation(DateTime *dateTime);
void DateTimeSet(DateTime *dateTime, int year, int month, int day, int weekDay, int hour, int minute, int second);

bool UDPbitStatus(byte data, int bytePos);
