#include "Arduino.h"
#include <EEPROM.h>
#include "eepromEvent.h"
#include "Basic.h"

EEPROMClass eeprom;

void FirstScan(Device *device) {
	// Bajty EEPROM //
	// 0 - 6 		: 	strefa[..] wymagana temperatura
	// 50 - 57		:   zone0 18b20 device address
	// 58 - 55		:   zone1 18b20 device address
	// 66 - 73		:   zone2 18b20 device address
	// 74 - 81		:   zone3 18b20 device address
	// 82 - 89		:   zone4 18b20 device address
	// 90 - 97		:   zone5 18b20 device address
	// 98 - 105		:   zone6 18b20 device address

	eeprom.begin(1024);						// EEPROM begin
	delay(100);
	// Get required temperatures
	for (int i=0; i<ZONE_QUANTITY; i++)
		device->zone[i].reqTemp = (float)eeprom.read(i)/2;

	// Get 18b20 device addresses
	int offset = 50;						// first byte of DS18b20 device addresses
	for (int i=0; i<ZONE_QUANTITY; i++)
		for (int j=0; j<8; j++)
			device->zone[i].deviceAddress[j] = eeprom.read(offset+8*i+j);

	eeprom.commit();
}

void EEpromWrite(int pos, int value) {
	eeprom.begin(1024);										// EEPROM begin
	eeprom.write(pos,value);
	eeprom.commit();
}

void TMPwriteValueToEEPROM() {
	int req[7];
	DeviceAddress tempDeviceAddress;

	//Zone reqTemp
//	req[0] = 1;
//	req[1] = 2;
//	req[2] = 3;
//	req[3] = 4;
//	req[4] = 5;
//	req[5] = 6;
//	req[6] = 7;
//	for (int i=0; i<7; i++) {
//		eeprom.begin(1024);
//		delay(100);
//		eeprom.write(i,req[i]);
//		eeprom.commit();
//	}

	int offset = 50;
	int zoneNo;

	// Zone 0 - Salon, kuchnia, klatka, przedpokój
	// [109] 	40-2-0-7-138-79-1-35	0,99	0,5
	zoneNo = 0;
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 2;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 138;
	tempDeviceAddress[5] = 79;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 35;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*zoneNo+i,tempDeviceAddress[i]);
		eeprom.commit();
	}
	delay(100);

	// Zone 1 - Pralnia, warsztat
	// [107] 	40-2-0-7-104-63-1-29	0,995	0
	zoneNo = 1;
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 2;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 104;
	tempDeviceAddress[5] = 63;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 29;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*zoneNo+i,tempDeviceAddress[i]);
		eeprom.commit();
	}
	delay(100);

	// Zone 2 - £azienka dó³,gabinet
	// [120] 	40-255-2-62-96-23-5-142	1,07	-4,5
	zoneNo = 2;
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 255;
	tempDeviceAddress[2] = 2;
	tempDeviceAddress[3] = 62;
	tempDeviceAddress[4] = 96;
	tempDeviceAddress[5] = 23;
	tempDeviceAddress[6] = 5;
	tempDeviceAddress[7] = 142;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*zoneNo+i,tempDeviceAddress[i]);
		eeprom.commit();
	}
	delay(100);

	// Zone 3 - rodzice
	// [111] 	40-2-0-7-201-119-1-173	0,98	2,1
	zoneNo = 3;
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 2;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 201;
	tempDeviceAddress[5] = 119;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 173;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*zoneNo+i,tempDeviceAddress[i]);
		eeprom.commit();
	}
	delay(100);

	// Zone 4 - Natalia
	// [110] 	40-2-0-7-38-42-1-203	0,985	0
	zoneNo = 4;
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 2;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 38;
	tempDeviceAddress[5] = 42;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 203;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*zoneNo+i,tempDeviceAddress[i]);
		eeprom.commit();
	}
	delay(100);

	// Zone 5 - Karolina
	//[106] 	40-2-0-7-192-72-1-22	0,995	-1,2
	zoneNo = 5;
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 2;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 192;
	tempDeviceAddress[5] = 72;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 22;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*zoneNo+i,tempDeviceAddress[i]);
		eeprom.commit();
	}
	delay(100);

	// Zone 6 - Laz Gora
	// [115] 	40-7-0-7-141-124-1-202	0,975	1
	zoneNo = 6;
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 7;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 141;
	tempDeviceAddress[5] = 124;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 202;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*zoneNo+i,tempDeviceAddress[i]);
		eeprom.commit();
	}
	delay(100);
}

