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

class Communication {
public:
	Communication(boolean start);
	virtual ~Communication();
	void WiFi_init();
	void run();

//private:
//	void UDPsendStandardFrame(byte data[128]);
//	byte* UDPread();
//	void UDPsendDiagnoseFrame();
};

bool WiFi_conectionCheck();

#endif /* COMMUNICATION_H_ */
