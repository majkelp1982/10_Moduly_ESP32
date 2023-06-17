#include "Arduino.h"
#include <Basic.h>
#include <OTA.h>
#include <Status.h>
#include <Communication.h>
#include "Module.h"
#include "WebService.h"

#define MODULE_TYPE			14
#define MODULE_NO			0
#define SERIAL_BAUD			115200

Basic basic(MODULE_TYPE,MODULE_NO, "modul_ogrzewanie", SERIAL_BAUD);
OTA ota(true);
Status status(false);
Communication communication(false);

void setup()
{
	communication.WiFi_init();
	ota.init();
	module_init();
	Serial.println("Start");
	addLog("Inicjalizacja zakoñczona");
	basic.WDT_init();
	webService_setup();
}

void loop()
{
	ota.client();
	basic.run();
	communication.run();
	status.printStatus(10);
	webService_run();
	module();
}
