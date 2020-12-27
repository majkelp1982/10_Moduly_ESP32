#include "Basic.h"
#include <Arduino.h>

//Variables
unsigned long dateTimeCalculationMillis = 0;

void Serial_Setup(int baud) {
	//Serial init
    Serial.begin(baud);									//Serial port for debug only. No needed. Disable to spare memory afterwards.
    Serial.println();
    Serial.print("Start ver.");
    Serial.println(Firm_VERSION);
}

void Pin_Setup() {
}

bool Sleep(unsigned long *lastMillis, int seconds) {
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

void DateTimeSet(DateTime *dateTime, int year, int month, int day, int weekDay, int hour, int minute, int second) {
	dateTime->year = year;
	dateTime->month = month;
	dateTime->day = day;
	dateTime->weekDay = weekDay;
	dateTime->hour = hour;
	dateTime->minute = minute;
	dateTime->second = second;
}

void DateTimeCalculation(DateTime *dateTime) {
	if (Sleep(&dateTimeCalculationMillis, 1)) return;

	dateTime->second+=1;
	if (dateTime->second >= 60) {
		dateTime->second -= 60;
		dateTime->minute++;
	}

	if (dateTime->minute == 60) {
		dateTime->minute = 0;
		dateTime->hour++;
	}

	if (dateTime->hour == 24) {
		dateTime->hour = 0;
	}
}

bool UDPbitStatus(byte data, int bytePos) {
	if (((data >> bytePos) & 1) == 1) return true;
	else return false;
}

