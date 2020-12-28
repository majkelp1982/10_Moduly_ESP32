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
	byte data[128];
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
void setUDPdata(int frameNo, byte data[128], int length);

#endif /* COMMUNICATION_H_ */
