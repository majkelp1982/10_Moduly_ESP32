#include "Module.h"

//PINS
//BME280
#define CS_BME280_CZERPNIA 					13
#define CS_BME280_WYRZUTNIA 				14
#define CS_BME280_NAWIEW	 				27
#define CS_BME280_WYWIEW	 				26

//SPI
#define SPI_SCK 							18
#define SPI_MISO 							19
#define SPI_MOSI							23


struct SensorBME280 {
	float temperature = 0.0f;
	int pressure = 0;
	int humidity = 0;
	Adafruit_BME280 interface;
	unsigned int faultyReadings = 0;
};

struct Device {
	SensorBME280 sensorsBME280[4];
};

Device device;
DataRead UDPdata;

//Functions
void outputs();
void readSensors();
void statusUpdate();
void readUDPdata();

//Variables
Adafruit_BME280 bme1(CS_BME280_CZERPNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme2(CS_BME280_WYRZUTNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme3(CS_BME280_NAWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme4(CS_BME280_WYWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);

//Delays
unsigned long readSensorMillis = 0;

void module_init() {
	//Set CS pins
	device.sensorsBME280[0].interface= bme1;
	device.sensorsBME280[1].interface= bme2;
	device.sensorsBME280[2].interface= bme3;
	device.sensorsBME280[3].interface= bme4;
	//initialization
	for (int i=0; i<4; i++)
		device.sensorsBME280[i].interface.begin();
}

void module() {
	readSensors();
	readUDPdata();
	outputs();
	statusUpdate();
}

void outputs() {
}

void readSensors() {
	if (sleep(&readSensorMillis, 5)) return;
	for (int i=0; i<4; i++) {
		device.sensorsBME280[i].temperature = device.sensorsBME280[i].interface.readTemperature();
		device.sensorsBME280[i].pressure = (int)(device.sensorsBME280[i].interface.readPressure()/100);
		device.sensorsBME280[i].humidity = (int)device.sensorsBME280[i].interface.readHumidity();
		//FIXME
		if (device.sensorsBME280[i].temperature>35
				|| device.sensorsBME280[i].temperature<10
				|| device.sensorsBME280[i].pressure>1050
				|| device.sensorsBME280[i].pressure<970
				|| device.sensorsBME280[i].humidity>100
				|| device.sensorsBME280[i].humidity<15)
			device.sensorsBME280[i].faultyReadings++;
	}
}

void statusUpdate() {
	String status;
	status = "MAIN\n";
	status +="Czerpnia:\t T="; status +=device.sensorsBME280[0].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[0].humidity;
	status +="[%]\tP="; status +=(int)device.sensorsBME280[0].pressure;status +="[hPa] Faulty="; status +=(int)device.sensorsBME280[0].faultyReadings ;status +="\n";
	status +="Wyrzutnia:\t T="; status +=device.sensorsBME280[1].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[1].humidity;
	status +="[%]\tP="; status +=(int)device.sensorsBME280[1].pressure;status +="[hPa] Faulty="; status +=(int)device.sensorsBME280[1].faultyReadings ;status +="\n";
	status +="Nawiew:\t\t T="; status +=device.sensorsBME280[2].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[2].humidity;
	status +="[%]\tP="; status +=(int)device.sensorsBME280[2].pressure;status +="[hPa] Faulty="; status +=(int)device.sensorsBME280[2].faultyReadings ;status +="\n";
	status +="Wywiew:\t\t T="; status +=device.sensorsBME280[3].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[3].humidity;
	status +="[%]\tP="; status +=(int)device.sensorsBME280[3].pressure;status +="[hPa] Faulty="; status +=(int)device.sensorsBME280[3].faultyReadings ;status +="\n";
	setStatus(status);
}

void readUDPdata() {
	UDPdata = getDataRead();
	if (!UDPdata.newData) return;
	resetNewData();
	Serial.printf("\nUDP [%i][%i][%i]\t\tDATA",UDPdata.deviceType,UDPdata.deviceNo,UDPdata.frameNo);
	for (int i=0; i<UDPdata.length;i++)
		Serial.printf("[%i]",UDPdata.data[i]);
}



