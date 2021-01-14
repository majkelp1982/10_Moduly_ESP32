#include "Arduino.h"
#include <Basic.h>
#include <OTA.h>
#include <Status.h>
#include <Communication.h>
#include "Module.h"

#define MODULE_TYPE			13
#define MODULE_NO			0
#define SERIAL_BAUD			921600

Basic basic(MODULE_TYPE,MODULE_NO,SERIAL_BAUD);
OTA ota(true);
Status status(true);
Communication communication(true);

void setup()
{
	communication.WiFi_init();
	ota.init();
	module_init();
	status.addLog("Inicjalizacja zako�czona");
}

void loop()
{
	ota.client();
	if (!ota.hasStarted()) {
		basic.run();
		communication.run();
		status.printStatus(10);
		module();
	}
}
