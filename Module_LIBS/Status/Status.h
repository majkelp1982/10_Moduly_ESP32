/*
 * Status.h
 *
 *  Created on: 23 gru 2020
 *      Author: spoma
 */

#ifndef STATUS_H_
#define STATUS_H_

#include "Arduino.h"
#include <Basic.h>
#include <Communication.h>

class Status {
public:
	Status(boolean start);
	virtual ~Status();
	void printStatus(int delay);
	void addLog(String log);
};

String getHTMLStatus();
void setStatus(String text);

#endif /* STATUS_H_ */
