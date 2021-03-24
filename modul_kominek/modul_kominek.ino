#include "Arduino.h"
#include <Basic.h>
#include <OTA.h>
#include <Status.h>
#include <Communication.h>
#include "Module.h"

#include <SH1106.h>
//OLED
#define OLED_RESET  19	// RESET
#define OLED_DC     21	// Data/Command
#define OLED_CS     5	// Chip select
#define OLED_SCK	18	//
#define OLED_SDA	23	//


#define MODULE_TYPE			5
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
	addLog("Inicjalizacja zakoñczona");
	basic.WDT_init();
}

void loop()
{
	ota.client();
	basic.run();
	communication.run();
	status.printStatus(1);
	module();
}
