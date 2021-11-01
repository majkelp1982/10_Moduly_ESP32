#include "Arduino.h"
#include <Basic.h>
#include <Status.h>
#include <Communication.h>
#include "Module.h"
#include "WebService.h"

#define MODULE_TYPE			10
#define MODULE_NO			0
#define SERIAL_BAUD			921600

Basic basic(MODULE_TYPE,MODULE_NO,"modul_komfort", SERIAL_BAUD);
Status status(true);
Communication communication(true);

void setup()
{
	communication.WiFi_init();

	webService_setup();
	module_init();
	addLog("Inicjalizacja zakoñczona");
	basic.WDT_init();
}

void loop()
{
	basic.run();
	communication.run();
	status.printStatus(10);
	module();
	webService_run();
}
