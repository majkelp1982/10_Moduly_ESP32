#include <OneWire.h>
#include <DallasTemperature.h>
#include "Basic.h"
#include "Module.h"

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);					// initialized 1-WIRE for buffers

unsigned long dallasReadMillis = 0;

void DALLAS18b20Read (Device *device, Zone *zone);
void DALLAS18b20ReadTemperatures(Device *device);

void Module_init() {
	sensors.begin();
	Serial.println("Module_init");

	sensors.setResolution(10);
	sensors.setWaitForConversion(false);
}

void Module(Device *device) {

	// Get actual temperatures
	DALLAS18b20ReadTemperatures(device);
}

void DALLAS18b20Read (Device *device, Zone *zone) {
	DeviceAddress deviceAddress;
	for (int i=0; i<8; i++) deviceAddress[i] = zone->deviceAddress[i];
	float tempC = 0;
	float a=0;
	float b=0;

	if (memcmp(device->zone[0].deviceAddress,deviceAddress,8) == 0) {a=0.99;	b=(0.5+2.8);}		// Zone 0 - Salon, kuchnia, klatka, przedpokój
	if (memcmp(device->zone[1].deviceAddress,deviceAddress,8) == 0) {a=0.995;	b=(0+2.2);}			// Zone 1 - Pralnia, warsztat
	if (memcmp(device->zone[2].deviceAddress,deviceAddress,8) == 0) {a=1.07;	b=(-4.5+2.7);}		// Zone 2 - £azienka dó³,gabinet
	if (memcmp(device->zone[3].deviceAddress,deviceAddress,8) == 0) {a=0.98;	b=(2.1+4.4);}		// Zone 3 - rodzice
	if (memcmp(device->zone[4].deviceAddress,deviceAddress,8) == 0) {a=0.985; 	b=(0+3.8);}			// Zone 4 - Natalia
	if (memcmp(device->zone[5].deviceAddress,deviceAddress,8) == 0) {a=0.975; 	b=(1+2.5);}			// Zone 5 - Karolina
	if (memcmp(device->zone[6].deviceAddress,deviceAddress,8) == 0) {a=0.995; 	b=(-1.2+4.7);}		// Zone 6 - Laz Gora

	tempC = sensors.getTempC(deviceAddress);

	if ((tempC<5) || (tempC>100)) {
		zone->errorCount++;
		//zapisz liczbe maksymalna zlych odczytow z rzedu
		if (zone->maxErrorCount < zone->errorCount) zone->maxErrorCount = zone->errorCount;
		//po trzech probach nie udanych wysli 100.00 stC
		if (zone->errorCount > 50) zone->isTemp = 10.00;
	}
	else {
		//przelicz temperature wedlug krzywej
		tempC = a * tempC + b;						// Temperature compensation
		//zerowanie liczby bledow
		zone->errorCount = 0;

		zone->isTemp = tempC;

	}
}


void DALLAS18b20ReadTemperatures(Device *device)
{
	if (Sleep(&dallasReadMillis, DELAY_DS18B20_READ)) return;

	//DALLAS18b20ReadDeviceAdresses();

	// Get actual temperatures and compensate
	for (int i=0; i<ZONE_QUANTITY; i++) {
		DALLAS18b20Read(device, &device->zone[i]);
	}
	sensors.requestTemperatures();				// Request temperature
}

void DALLAS18b20ReadDeviceAdresses() {
	DeviceAddress tempDeviceAddress;
	for (int i=0; i<sensors.getDeviceCount(); i++) {
		sensors.getAddress(tempDeviceAddress,i);
		Serial.printf("[%d] Adres [%d][%d][%d][%d][%d][%d][%d][%d]",i,tempDeviceAddress[0],tempDeviceAddress[1],tempDeviceAddress[2],tempDeviceAddress[3],tempDeviceAddress[4],tempDeviceAddress[5],tempDeviceAddress[6],tempDeviceAddress[7]);
		Serial.println();
	}
}
