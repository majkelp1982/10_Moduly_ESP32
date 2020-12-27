#include "Arduino.h"
// Definitions for modul wentylacji

#pragma once
//Firmware version
#define Firm_VERSION 						20062318	// rok miesiac dzien godzina
#define DEVICE_TYP							3			// modul ogrzewania
#define DEVICE_NO							0			// numer modulu ogrzewania

//NETWORK
//#define WIFI_SSID 							"Majkel"
//#define WIFI_PSWD 							"12345678"

//#define WIFI_SSID 							"PYUR D1BCC"
//#define WIFI_PSWD 							"XzJznewy4j2U"

//#define WIFI_SSID 							"HOTSPLOTSComfort"
//#define WIFI_PSWD 							""

#define WIFI_SSID 							"PYUR D3E5E"
#define WIFI_PSWD 							"AWX628peadvS"

//#define WIFI_SSID 							"NOKIALumia630"
//#define WIFI_PSWD 							"romek12333"

#define LOCAL_PORT							6000

// UDP
#define DELAY_BETWEEN_UDP_STANDARD			10		// delay between UDP send [s]
#define DELAY_BETWEEN_UDP_DIAGNOSE			30		// delay between UDP send [s]
#define FRAME_SIZE							15
#define FRAME_DIAGNOSE_SIZE					6

// DELAYS
#define DELAY_UDP_SAVE						120000

//PIN DEFINITION
#define FAN									15
#define PUMP								0
//ALERT
#define HUMIDITY_ALERT						75			// Humidity limit to turn fun on

struct AirSupply {
	// flaps true = open
	bool Salon1 = false;
	bool Salon2 = false;
	bool Gabinet = false;
	bool Warsztat = false;
	bool Rodzice = false;
	bool Natalia = false;
	bool Karolina = false;
};

struct AirExhaust {
	// flaps true = open
	bool Przedpokoj = false;
	bool Pralnia = false;
	bool Garderoba = false;
	bool LazDol1 = false;
	bool LazDol2 = false;
	bool LazGora1 = false;
	bool LazGora2 = false;
};

//Structure
struct Diagnose {
	byte ip[4];
	byte wifiConnectionInterrupt = 0;
	byte wifiConnected = false;
};

struct Device {
	Diagnose diagnose;
	bool fan = false;
	bool pump = false;
	bool normalOn = false;
	bool humidityAlert = false;
	int hour[12];
	bool superHeating = false;
	bool activeCooling = false;
	AirSupply airSupply;
	AirExhaust airExhaust;
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

struct Hum {
	int is = 0;
	unsigned long lastUpdate = 0;
};

struct Humidity {
	Hum salon;
	Hum lazDol;
	Hum pralnia;
	Hum rodzice;
	Hum natalia;
	Hum karolina;
	Hum lazGora;

};

struct AirHeatingActive {
	bool salon=false;
	bool gabinet=false;
	bool warsztat=false;
	bool rodzice=false;
	bool Natalia=false;
	bool Karolina=false;
};

struct Temp {
	float is = 0;
	float req = 0;
	unsigned long lastUpdate = 0;
};

struct Temperature {
	Temp salon;
	Temp lazDol;
	Temp pralnia;
	Temp rodzice;
	Temp natalia;
	Temp karolina;
	Temp lazGora;
};

void Pin_Setup();
void Serial_Setup(int baud);
bool Sleep(unsigned long *lastMillis, int seconds);

void DateTimeCalculation(DateTime *dateTime);
void DateTimeSet(DateTime *dateTime, int year, int month, int day, int weekDay, int hour, int minute, int second);
bool UDPbitStatus(byte data, int bytePos);
