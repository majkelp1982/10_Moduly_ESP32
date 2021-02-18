#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SH1106.h>

#pragma once

//DEFINES
//MODULES ID
#define ID_MOD_MAIN							1
#define ID_MOD_HEATING						14

//Alarms
#define WARN								1
#define ALARM								2

//DALLAS ID
#define ID_INLET							0
#define ID_OUTLET							1
#define ID_CHIMNEY							2

//DALLAS 1-wire
#define DELAY_SENSORS_READ					10			// delay between sensors reading [s]
#define PIN_DS18B20							21

//PINS
//OLED
#define OLED_RESET  19	// RESET
#define OLED_DC     21	// Data/Command
#define OLED_CS     5	// Chip select
#define OLED_SCK	18	//
#define OLED_SDA	23	//

//rotary encoder
#define inCLK					16
#define inDT					17
#define inSW 					22

//Alarm spiker
#define outSPIKER				15

struct Thermometer {						// Thermometer
	DeviceAddress deviceAddress;
	float isTemp=0;							// measured temperature
	int errorCount = 0;						// number of faulty reading
	int maxErrorCount = 0;
};

struct HeatingModule {
	bool pumpInHouse = false;
	float tBuffCOgora = 0;
	unsigned long paramsLastTimeRead = 0;
};

struct Message {
	String message;
	int curr = 100;
	int goal = 100;
};

struct Info {
	int type;		// 1-warning	2- ALARM!!!
	String mess1;
	String mess2;
	String mess3;
	bool active;
};

struct Display {
	String header;
	Message top;
	Message bottom;
	Info info[5];
};

struct Device {
	int mode = 0;							//	0-CZUWANIE, 1-ROZPALANIE, 2-GRZANIE, 3-GASZENIE
	bool alarm = false;
	bool warning = false;
	bool pump = false;
	bool fireAlarm = false;
	Thermometer thermo[3];
	int reqTemp;
	int throttle = 0;
	int minutesOnFire = 0;
	float mode1StartTemperature = 0;
};

void module_init();
void module();


