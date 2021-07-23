#include "Arduino.h"
#include <Basic.h>
#include <OTA.h>
#include <Status.h>
#include <Communication.h>
#include "Module.h"

#define MODULE_TYPE			10
#define MODULE_NO			0
#define SERIAL_BAUD			921600

Basic basic(MODULE_TYPE,MODULE_NO,"modul_komfort", SERIAL_BAUD);
OTA ota(true);
Status status(true);
Communication communication(false);

void setup()
{
	communication.WiFi_init();
	ota.init();
	module_init();
	addLog("Inicjalizacja zakoñczona");
	basic.WDT_init();
}

void loop()
{
	ota.client();
	basic.run();
	communication.run();
	status.printStatus(10);
	module();
}
