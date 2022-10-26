#include "Basic.h"

int _moduleType = 0;
int _moduleNo = 0;
String _moduleName = "";
Diagnose diagnose;
DateTime dateTime;

Basic::Basic(int moduleType, int moduleNo, String modulName, int serialBaud) {
	_moduleType = moduleType;
	_moduleNo = moduleNo;
	_moduleName = modulName;
	serialSetup(serialBaud);
}

Basic::~Basic() {
}

void Basic::run() {
	dateTimeCalculation();
	esp_task_wdt_reset();
}

void Basic::WDT_init() {
	esp_task_wdt_init(60, true); //enable panic so ESP32 restarts
	esp_task_wdt_add(NULL); //add current thread to WDT watch
}

void Basic::serialSetup(int baud) {
	//Serial init
    Serial.begin(baud);									//Serial port for debug only. No needed. Disable to spare memory afterwards.
    String text = "Start\nVersion ";
    text += FIRMWARE_VERSION;
    Serial.println(text);
}

void dateTimeSet(int year, int month, int day, int weekDay, int hour, int minute, int second) {
	dateTime.year = year;
	dateTime.month = month;
	dateTime.day = day;
	dateTime.weekDay = weekDay;
	dateTime.hour = hour;
	dateTime.minute = minute;
	dateTime.second = second;
}

void Basic::dateTimeCalculation() {
	if (sleep(&dateTimeCalculationMillis, 1)) return;

	dateTime.second+=1;
	if (dateTime.second >= 60) {
		dateTime.second -= 60;
		dateTime.minute++;
	}

	if (dateTime.minute == 60) {
		dateTime.minute = 0;
		dateTime.hour++;
	}

	if (dateTime.hour == 24) {
		dateTime.hour = 0;
	}
}

bool UDPbitStatus(byte data, int bytePos) {
	if (((data >> bytePos) & 1) == 1) return true;
	else return false;
}

//Help functions
byte get10Temp(float temp){
	byte value;
	// when value under 0 add sign bit
	if (temp<0) value=(1<<7);
	else value = 0;
	return (byte)(value+abs(temp));
}

byte get01Temp(float temp) {
	byte temp10 = get10Temp(abs(temp));
	return (byte) ((abs(temp)*10)-(temp10*10));
}


int getModuleType() {
	return _moduleType;
}

int getModuleNo() {
	return _moduleNo;
}

String getModuleName() {
	return _moduleName;
}

bool sleep(unsigned long *lastMillis, int seconds) {
	// if sleep active return true
	bool status = true;
	int ms = 1000 * seconds;
	unsigned long currentMillis = millis();
	if ((currentMillis-*lastMillis) >= ms) {
		*lastMillis = currentMillis;
		status = false;
	}
	else status = true;
	return status;
}

DateTime getDateTime() {
	return dateTime;
}

Diagnose getDiagnose() {
	return diagnose;
}

void setDiagnose(Diagnose diag) {
	diagnose = diag;
}

void getPinState(PinInput *pinInput, int pin, bool edge) {
	unsigned long currentMillis = millis();
	if ((!edge) && (currentMillis-pinInput->lastRead<5))
		return;
	bool pinState = !digitalRead(pin);
	pinInput->isState = pinState;
	if ((edge) && (pinInput->isState) && (pinInput->lastState)) {
		pinInput->isState = 0;
		pinInput->lastState = 1;
	}
	else
		pinInput->lastState  = pinState;
	pinInput->lastRead = currentMillis;
}

String addStatus(String message, String value, String unit) {
	String status;
	status += "\t";
	status += message;
	status += ":";
	status += value;
	if (unit != "") {
		status += "[";
		status += unit;
		status += "]";
	}
	return status;
}

String addStatus(String message, int value, String unit) {
	return addStatus(message, (String)value, unit);
}

String addStatus(String message, unsigned int value, String unit) {
	return addStatus(message, (String)value, unit);
}

String addStatus(String message,  byte value, String unit) {
	return addStatus(message, (String)value, unit);
}

String addStatus(String message, bool value) {
	return addStatus(message, (value)?"TAK":"NIE", "");
}

String addStatus(String message, float value, String unit) {
	return addStatus(message, (String)value, unit);
}

