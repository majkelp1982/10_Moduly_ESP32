#include <Arduino.h>
#include "Basic.h"

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
	//CIRCUT RELAYS
	pinMode (ZONE0,OUTPUT);					digitalWrite(ZONE0, HIGH);
	pinMode (ZONE1,OUTPUT);					digitalWrite(ZONE1, HIGH);
	pinMode (ZONE2,OUTPUT);					digitalWrite(ZONE2, HIGH);
	pinMode (ZONE3,OUTPUT);					digitalWrite(ZONE3, HIGH);
	pinMode (ZONE4,OUTPUT);					digitalWrite(ZONE4, HIGH);
	pinMode (ZONE5,OUTPUT);					digitalWrite(ZONE5, HIGH);
	pinMode (ZONE6,OUTPUT);					digitalWrite(ZONE6, HIGH);

	//CIRCULATION PUMP
	pinMode (PUMP_IN_HOUSE,OUTPUT); 		digitalWrite(PUMP_IN_HOUSE, HIGH);
	pinMode (PUMP_UNDER_HOUSE,OUTPUT); 		digitalWrite(PUMP_UNDER_HOUSE, HIGH);

	//VALVES
	pinMode (VALVE_3WAY_CO,OUTPUT); 		digitalWrite(VALVE_3WAY_CO, HIGH);
	pinMode (VALVE_3WAY_CWU,OUTPUT); 		digitalWrite(VALVE_3WAY_CWU, HIGH);
	pinMode (VALVE_BYPASS_OPEN,OUTPUT); 	digitalWrite(VALVE_BYPASS_OPEN, HIGH);
	pinMode (VALVE_BYPASS_CLOSE,OUTPUT); 	digitalWrite(VALVE_BYPASS_CLOSE, HIGH);

	//HEATING PUMP
	pinMode (RELAY_HEAT_PUMP_ON,OUTPUT); digitalWrite(RELAY_HEAT_PUMP_ON, HIGH);

	//ANTYLEGIONELLIA
	pinMode (RELAY_ANTILEGIONELLIA,OUTPUT); digitalWrite(RELAY_ANTILEGIONELLIA, HIGH);
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

