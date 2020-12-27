#include <Arduino.h>
#include "Basic.h"

//Functions
void Outputs(Device *device);
void ReadSensors(Device *device);

//Variables
Adafruit_BME280 bme1(CS_BME280_CZERPNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme2(CS_BME280_WYRZUTNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme3(CS_BME280_NAWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme4(CS_BME280_WYWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);

//Delays
unsigned long readSensorMillis = 0;

void Module_init(Device *device) {
	//Set CS pins
	device->sensorsBME280[0].interface= bme1;
	device->sensorsBME280[1].interface= bme2;
	device->sensorsBME280[2].interface= bme3;
	device->sensorsBME280[3].interface= bme4;
	//initialization
	for (int i=0; i<4; i++)
		device->sensorsBME280[i].interface.begin();
}

void Module(Device *device, DateTime datTime) {
	ReadSensors(device);
	Outputs(device);
}

void Outputs(Device *device) {
}

void ReadSensors(Device *device) {
	if (Sleep(&readSensorMillis, 2)) return;
	for (int i=0; i<4; i++) {
		device->sensorsBME280[i].temperature = device->sensorsBME280[i].interface.readTemperature();
		device->sensorsBME280[i].pressure = (int)(device->sensorsBME280[i].interface.readPressure()/100);
		device->sensorsBME280[i].humidity = (int)device->sensorsBME280[i].interface.readHumidity();
		//FIXME
		if (device->sensorsBME280[i].temperature>35
				|| device->sensorsBME280[i].temperature<10
				|| device->sensorsBME280[i].pressure>1050
				|| device->sensorsBME280[i].pressure<970
				|| device->sensorsBME280[i].humidity>100
				|| device->sensorsBME280[i].humidity<15)
			device->sensorsBME280[i].faultyReadings++;
		//FIXME
//		Serial.printf("[%d] T=%.1f P=%d H=%d\n",i,device->sensorsBME280[i].temperature,device->sensorsBME280[i].pressure,device->sensorsBME280[i].humidity);
	}
	//FIXME
//	Serial.println();
}


