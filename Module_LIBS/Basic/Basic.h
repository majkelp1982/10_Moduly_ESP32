#ifndef BASIC_H_
#define BASIC_H_
#include "Arduino.h"
#include <esp_task_wdt.h>

#define VERSION				"2021.05.17"
#define BETA_VERSION		"_airFlow3"

#define FIRMWARE_VERSION	VERSION BETA_VERSION


struct Diagnose {
	byte ip[4];
	byte wifiConnectionInterrupt = 0;
	byte wifiConnected = false;
	byte signal = 0;
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

struct PinInput {
	bool lastState = false;
	bool isState = false;
	unsigned long lastRead = 0;
};

class Basic {
public:
	Basic();
	Basic(int moduleType, int moduleNo, String modulName, int serialBaud);
	~Basic(void);
	void run();
	void WDT_init();
private:
	void dateTimeCalculation();
	void serialSetup(int baud);

	//Millis
	unsigned long dateTimeCalculationMillis;
};

bool sleep(unsigned long *lastMillis, int seconds);
int getModuleType();
int getModuleNo();
String getModuleName();
DateTime getDateTime();
Diagnose getDiagnose();
void setDiagnose(Diagnose diag);
void dateTimeSet(int year, int month, int day, int weekDay, int hour, int minute, int second);
bool UDPbitStatus(byte data, int bytePos);
void getPinState(PinInput *pinInput, int pin, bool edge);

//Help functions get byte from float number
byte get10Temp(float temp);
byte get01Temp(float temp);

#endif /* BASIC_H_ */
