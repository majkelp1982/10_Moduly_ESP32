/*
 * OTA.h
 *
 *  Created on: 23 gru 2020
 *      Author: spoma
 */

#ifndef OTA_H_
#define OTA_H_

#include "Arduino.h"
#include "Status.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

class OTA {
public:
	OTA(unsigned long version);
	~OTA();
	void init();
	void client();
};

#endif /* OTA_H_ */
