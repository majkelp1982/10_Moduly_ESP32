#include "Arduino.h"
#include <Basic.h>
#include <OTA.h>
#include <Status.h>
#include <Communication.h>
#include "Module.h"

#define FIRMWARE_VERSION	2020122717
#define MODULE_TYPE			13
#define MODULE_NO			0
#define SERIAL_BAUD			921600

Basic basic(FIRMWARE_VERSION,MODULE_TYPE,MODULE_NO,SERIAL_BAUD);
OTA ota(FIRMWARE_VERSION);
Status status(true);
Communication communication(true);

void setup()
{
	communication.WiFi_init();
	ota.init();
	module_init();
	status.addLog("Inicjalizacja zakoñczona");
}

void loop()
{
	basic.run();
	communication.run();
	ota.client();
	status.printStatus(10);
	module();
}
