/*
 * Communication.h
 *
 *  Created on: 26 gru 2020
 *      Author: spoma
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include "Arduino.h"
#include "Communication.h"
#include "Basic.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Status.h>
#include <EEPROM.h>

#define MAX_DATA_SIZE		128

struct DataRead {
	boolean newData=false;
	int deviceType= 0;
	int deviceNo = 0;
	int frameNo = 0;
	int length = 0;
	byte data[128];
};

struct DataWritte {
	int length = 0;
	int frameNo = 0;
	byte data[MAX_DATA_SIZE];
	byte lastData[MAX_DATA_SIZE];
};

class Communication {
public:
	Communication(boolean start);
	virtual ~Communication();
	void WiFi_init();
	void run();
};

bool WiFi_conectionCheck();
DataRead getDataRead();
void resetNewData();
void forceStandardUDP();
void setUDPdata(int frameNo, byte *data, int length);
void EEpromScan(byte EEpromData[], int size);
void EEpromWrite(int pos, int value);

#endif /* COMMUNICATION_H_ */
