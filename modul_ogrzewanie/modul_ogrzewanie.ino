#include "Arduino.h"
#include <Basic.h>
#include <OTA.h>
#include <Status.h>
#include <Communication.h>
#include "Module.h"

#define MODULE_TYPE			14
#define MODULE_NO			0
#define SERIAL_BAUD			921600

Basic basic(MODULE_TYPE,MODULE_NO, "modul_ogrzewanie", SERIAL_BAUD);
OTA ota(true);
Status status(true);
Communication communication(true);

void setup()
{
	communication.WiFi_init();
	ota.init();
	module_init();
	addLog("Inicjalizacja zako�czona");
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
