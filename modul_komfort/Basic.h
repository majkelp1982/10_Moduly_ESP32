// Definitions for modul komfortu

#pragma once
#include <DallasTemperature.h>

#pragma once
//Firmware version
#define Firm_VERSION 						2020012120	// rok miesiac dzien godzina
#define DEVICE_TYP							10			// modul ogrzewania
#define DEVICE_NO							0			// numer modulu ogrzewania

//NETWORK
#define WIFI_SSID 							"Majkel"
#define WIFI_PSWD 							"12345678"

//#define WIFI_SSID 							"PYUR D1BCC"
//#define WIFI_PSWD 							"XzJznewy4j2U"
//#define WIFI_SSID 							"PYUR D3E5E"
//#define WIFI_PSWD 							"AWX628peadvS"

//#define WIFI_SSID 							"HOTSPLOTSComfort"
//#define WIFI_PSWD 							""

//#define WIFI_SSID 							"NOKIALumia630"
//#define WIFI_PSWD 							"romek12333"

#define LOCAL_PORT							6000

// UDP
#define DELAY_BETWEEN_UDP_STANDARD			10		// delay between UDP send [s]
#define DELAY_BETWEEN_UDP_DIAGNOSE			30		// delay between UDP send [s]
#define FRAME_STANDARD_SIZE					30
#define FRAME_DIAGNOSE_SIZE					6

//DALLAS 1-wire
#define DELAY_DS18B20_READ					10//20		// delay between sensors reading [s]
#define ONE_WIRE_PIN						23

//CONST
#define	ZONE_QUANTITY						7		// quantity of zones

//Struktura strefy
struct Zone {
	DeviceAddress deviceAddress;			//Address DB18b20 1-wire
	int errorCount = 0;						// number of faulty reading
	int maxErrorCount = 0;						// number of faulty reading
	float isTemp=0;
	float reqTemp=0;
	int humidity=0;
};

struct Diagose {
	byte ip[4];
	byte wifiConnectionInterrupt = 0;
	byte wifiConnected = false;
};
//Structure
struct Device {
	Diagose diagnose;
	Zone zone[ZONE_QUANTITY];
	int sensorsCount = 0;
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
