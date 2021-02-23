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
#define PIN_DS18B20							13

//PINS
//OLED
#define OLED_RESET  19	// RESET
#define OLED_DC     21	// Data/Command
#define OLED_CS     5	// Chip select
#define OLED_SCK	18	//
#define OLED_SDA	23	//

//rotary encoder
#define pinCLK					16
#define pinDT					17
#define pinSW 					22

//Alarm spiker
#define pinSPIKER				0//TODO

//PUMP
#define pinPUMP					32

//THROTTLE
#define SERVO_FREQUENCY 					50 		// Hz
#define SERVO_CHANNEL 						2		// this variable is used to select the channel number
#define SERVO_RESOUTION 					8 		// resolution of the signal
#define THROTTLE_PIN						25 		// GPIO to PWM fan input


//Information number
#define INFO_MAX				6


struct Thermometer {						// Thermometer
	DeviceAddress deviceAddress;
	float isTemp=0;							// measured temperature
	int errorCount = 0;						// number of faulty reading
	int maxErrorCount = 0;
};

struct HeatingModule {
	bool pumpInHouse = false;
	int tBuffCOgora = 0;
	int tBuffCOdol = 0;
	unsigned long paramsLastTimeRead = 0;
};

struct Message {
	String message;
	int curr = 100;
	int goal = 100;
};

struct Info {
	int type = 0;		// 1-warning	2- ALARM!!!
	String mess1 = "";
	String mess2 = "";
	String mess3 = "";
	bool active = false;
};

struct Screen {
	String header;
	Message top;
	Message bottom;
	Info info[INFO_MAX];
	int activeScreen = 1;
};

struct Device {
	int mode = 0;							//	0-CZUWANIE, 1-ROZPALANIE, 2-GRZANIE, 3-GASZENIE
	bool alarm = false;
	bool warning = false;
	bool settingsActive = false;
	bool pump = false;
	bool buzzer = false;
	bool fireAlarm = false;
	Thermometer thermo[3];
	int reqTemp;
	int lowestTemp = 0;
	int throttle = 0;
	unsigned long startMillis = 0;
	int startTemp = 0;
	bool reqTempReached;
};

void module_init();
void module();


