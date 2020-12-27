#include "Arduino.h"
#include <EEPROM.h>
#include "eepromEvent.h"
#include "Basic.h"

EEPROMClass eeprom;

void FirstScan(Device *device) {
	// Bajty EEPROM //
	// 1 			: 	tylko taryfa II
	// 2			:   ogrzewanie
	// 3			:   nastawiona temperatura CO
	// 4			:   nastawiona temperatura CWU
	// 5			:	Nastawa temperatury alarmowej
	// 10 - 17		:   Bufor CO dol 18b20 device address
	// 18 - 25		:   Bufor CO srodek 18b20 device address
	// 26 - 33		:   Bufor CO gora 18b20 device address
	// 34 - 41		:   Bufor CWU dol 18b20 device address
	// 42 - 49		:   Bufor CWU srodek 18b20 device address
	// 50 - 57		:   Bufor CWU gora 18b20 device address
	// 58 - 65		:   zasilanie 18b20 device address
	// 66 - 73		:   powrot 18b20 device address
	// 74 - 81		:   dolneZrodlo 18b20 device address
	// 82 - 89		:   Kominek 18b20 device address
	// 90 - 97		:   Rozdzielacze 18b20 device address
	// 98 - 105		:   PowrotParter 18b20 device address
	// 106 - 113	:   PowrotPietro 18b20 device address

	Serial.println("EEPROM initialization of start values");

	eeprom.begin(1024);						// EEPROM begin
	delay(100);
	if (eeprom.read(1) == 0) device->cheapTariffOnly = false; else device->cheapTariffOnly = true;
	if (eeprom.read(2) == 0) device->heatingActivated = false; else device->heatingActivated = true;
	device->reqTempBuforCO = eeprom.read(3)/2.0;
	device->reqTempBuforCWU = eeprom.read(4)/2.0;
	device->heatPumpAlarmTemperature = eeprom.read(5);

	// Get 18b20 device addresses
	int offset = 10;
	for (int i=0; i<8; i++) {
		device->tBuffCOdol.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tBuffCOdol.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tBuffCOsrodek.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tBuffCOsrodek.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tBuffCOgora.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tBuffCOgora.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tBuffCWUdol.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tBuffCWUdol.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tBuffCWUsrodek.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tBuffCWUsrodek.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tBuffCWUgora.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tBuffCWUgora.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tZasilanie.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tZasilanie.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tPowrot.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tPowrot.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tDolneZrodlo.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tDolneZrodlo.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tKominek.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tKominek.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tRozdzielacze.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tRozdzielacze.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tPowrotParter.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tPowrotParter.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device->tPowrotPietro.deviceAddress[i] = eeprom.read(offset+i);
		Serial.printf("[%d]",device->tPowrotPietro.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;

	Serial.printf("Next free byte in EEPROM[%d]",offset);
	Serial.println();
	delay(1000);

	eeprom.commit();
}

void EEpromWrite(int pos, int value) {
	eeprom.begin(1024);										// EEPROM begin
	eeprom.write(pos,value);
	eeprom.commit();
}

void TMPwriteValueToEEPROM() {
	Serial.println("Save to EEPROM");
	unsigned char tempDeviceAddress[8];

	int offset = 10;
	int sensorNo;

	// sensorNo 0
	sensorNo = 0;
	// |18| [40] [2] [0] [7] [47] [39] [1] [204]  |	   tempBuforCOdol		|
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 2;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 47;
	tempDeviceAddress[5] = 39;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 204;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 1
	sensorNo = 1;
	// |19| [40] [7] [0] [7] [205] [124] [1] [251]|	   tempBuforCOsrodek	|
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 7;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 205;
	tempDeviceAddress[5] = 124;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 251;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 2
	sensorNo = 2;
	// |20|[40] [12] [1] [7] [112] [170] [1] [144]|	   tempBuforCOgora		|
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 12;
	tempDeviceAddress[2] = 1;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 112;
	tempDeviceAddress[5] = 170;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 144;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 3
	sensorNo = 3;
	//		|	21	| 		40-12-1-7-67-166-1-231			|	 tempBuforCWUdol(101)   |
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 12;
	tempDeviceAddress[2] = 1;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 67;
	tempDeviceAddress[5] = 166;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 231;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 4
	sensorNo = 4;
	//		|-------|---------------------------------------|---------------------------|
	//		|	23	| 		40-12-1-7-194-103-1-94			|	tempBuforCWUsrodek(102)	|
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 12;
	tempDeviceAddress[2] = 1;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 194;
	tempDeviceAddress[5] = 103;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 94;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 5
	sensorNo = 5;
	//		|-------|---------------------------------------|---------------------------|
	//		|	22	| 		40-12-1-7-225-170-1-19			|   tempBuforCWUgora(103)	|
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 12;
	tempDeviceAddress[2] = 1;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 225;
	tempDeviceAddress[5] = 170;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 19;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 6
	sensorNo = 6;
	// |7| [40] [12] [1] [7] [159] [197] [1] [74]|	 tempZrodla		|
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 12;
	tempDeviceAddress[2] = 1;
	tempDeviceAddress[3] = 7;
	tempDeviceAddress[4] = 159;
	tempDeviceAddress[5] = 197;
	tempDeviceAddress[6] = 1;
	tempDeviceAddress[7] = 74;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 7
	sensorNo = 7;
	// |1| [40][255] [161][100] [96][23][5] [48]	|    tempPoOdbiornikach		|
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 255;
	tempDeviceAddress[2] = 161;
	tempDeviceAddress[3] = 100;
	tempDeviceAddress[4] = 96;
	tempDeviceAddress[5] = 23;
	tempDeviceAddress[6] = 5;
	tempDeviceAddress[7] = 48;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 8
	sensorNo = 8;
	tempDeviceAddress[0] = 0;
	tempDeviceAddress[1] = 0;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 0;
	tempDeviceAddress[4] = 0;
	tempDeviceAddress[5] = 0;
	tempDeviceAddress[6] = 0;
	tempDeviceAddress[7] = 0;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 9
	sensorNo = 9;
	// |2| [40][255][21][68][96][23][5] [16]	|   tempWymiennikKominka    |
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 255;
	tempDeviceAddress[2] = 21;
	tempDeviceAddress[3] = 68;
	tempDeviceAddress[4] = 96;
	tempDeviceAddress[5] = 23;
	tempDeviceAddress[6] = 5;
	tempDeviceAddress[7] = 16;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 10
	sensorNo = 10;
	// |3|[40][255][154][228][161][22][3][252]		| tempZasilaniaRozdzielaczy |
	tempDeviceAddress[0] = 40;
	tempDeviceAddress[1] = 255;
	tempDeviceAddress[2] = 154;
	tempDeviceAddress[3] = 228;
	tempDeviceAddress[4] = 161;
	tempDeviceAddress[5] = 22;
	tempDeviceAddress[6] = 3;
	tempDeviceAddress[7] = 252;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 11
	sensorNo = 11;
	tempDeviceAddress[0] = 0;
	tempDeviceAddress[1] = 0;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 0;
	tempDeviceAddress[4] = 0;
	tempDeviceAddress[5] = 0;
	tempDeviceAddress[6] = 0;
	tempDeviceAddress[7] = 0;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

	// sensorNo 12
	sensorNo = 12;
	tempDeviceAddress[0] = 0;
	tempDeviceAddress[1] = 0;
	tempDeviceAddress[2] = 0;
	tempDeviceAddress[3] = 0;
	tempDeviceAddress[4] = 0;
	tempDeviceAddress[5] = 0;
	tempDeviceAddress[6] = 0;
	tempDeviceAddress[7] = 0;
	//Address is saving
	for (int i=0; i<8; i++) {
		eeprom.begin(1024);
		delay(100);
		eeprom.write(offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.printf("Byte[%d], value[%d]",offset+8*sensorNo+i,tempDeviceAddress[i]);
		Serial.println();
		eeprom.commit();
	}
	delay(100);

}

