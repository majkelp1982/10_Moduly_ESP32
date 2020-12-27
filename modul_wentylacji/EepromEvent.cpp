#include "Arduino.h"
#include <EEPROM.h>
#include "eepromEvent.h"

EEPROMClass eeprom;

void FirstScan(Device *device) {
	// Bajty EEPROM //
	// 0 - 11 		: 	device.hour[0] .. device.houre[11]
	eeprom.begin(1024);						// EEPROM begin
	delay(100);
	for (int i=0; i<12; i++)
		device->hour[i] = eeprom.read(i);
}

void EEpromWrite(int pos, int value) {
	eeprom.begin(1024);										// EEPROM begin
	eeprom.write(pos,value);
	eeprom.commit();
}
