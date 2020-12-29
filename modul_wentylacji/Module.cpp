#include "Module.h"
#include <EEPROM.h>

EEPROMClass eeprom;

Device device;
DataRead UDPdata;
Servo servo;

//Functions
void outputs();
void readSensors();
void statusUpdate();
void readUDPdata();
boolean normalMode();
void setUDPdata();
void bypassSetting();

//Variables
Adafruit_BME280 bme1(CS_BME280_CZERPNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme2(CS_BME280_WYRZUTNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme3(CS_BME280_NAWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme4(CS_BME280_WYWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);

//Delays
unsigned long readSensorMillis = 0;

void EEpromScan() {
	// Bajty EEPROM //
	// 0 - 11 		: 	device.hour[0] .. device.hour[11]
	eeprom.begin(1024);						// EEPROM begin
	delay(100);
	Serial.print("Getting data from EEprom");
	for (int i=0; i<12; i++)
		device.hour[i] = eeprom.read(i);
}

void EEpromWrite(int pos, int value) {
	eeprom.begin(1024);										// EEPROM begin
	eeprom.write(pos,value);
	eeprom.commit();
}

void module_init() {
	//Set CS pins
	device.sensorsBME280[0].interface= bme1;
	device.sensorsBME280[1].interface= bme2;
	device.sensorsBME280[2].interface= bme3;
	device.sensorsBME280[3].interface= bme4;
	//initialization
	for (int i=0; i<4; i++)
		device.sensorsBME280[i].interface.begin();

	//EEprom Scan
	EEpromScan();

	//Servo init
	servo.attach(PIN_SERVO);
}

void module() {
	readSensors();
	readUDPdata();
	//Modes
	bypassSetting();
	device.normalON = normalMode();
	device.fan = device.normalON | device.humidityAlert;

	setUDPdata();
	outputs();
	statusUpdate();
}

void bypassSetting() {
	//TODO
	device.bypassOpen = device.normalON;
}

boolean normalMode() {
	bool normalOn = false;
	DateTime dateTime = getDateTime();
	if (dateTime.hour == 0) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[0],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[0],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[0],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[0],4))) normalOn = true;
	}
	if (dateTime.hour == 1) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[0],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[0],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[0],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[0],0))) normalOn = true;
	}
	if (dateTime.hour == 2) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[1],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[1],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[1],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[1],4))) normalOn = true;
	}
	if (dateTime.hour == 3) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[1],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[1],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[1],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[1],0))) normalOn = true;
	}
	if (dateTime.hour == 4) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[2],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[2],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[2],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[2],4))) normalOn = true;
	}
	if (dateTime.hour == 5) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[2],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[2],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[2],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[2],0))) normalOn = true;
	}
	if (dateTime.hour == 6) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[3],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[3],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[3],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[3],4))) normalOn = true;
	}
	if (dateTime.hour == 7) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[3],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[3],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[3],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[3],0))) normalOn = true;
	}
	if (dateTime.hour == 8) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[4],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[4],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[4],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[4],4))) normalOn = true;
	}
	if (dateTime.hour == 9) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[4],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[4],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[4],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[4],0))) normalOn = true;
	}
	if (dateTime.hour == 10) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[5],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[5],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[5],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[5],4))) normalOn = true;
	}
	if (dateTime.hour == 11) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[5],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[5],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[5],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[5],0))) normalOn = true;
	}
	if (dateTime.hour == 12) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[6],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[6],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[6],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[6],4))) normalOn = true;
	}
	if (dateTime.hour == 13) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[6],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[6],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[6],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[6],0))) normalOn = true;
	}
	if (dateTime.hour == 14) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[7],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[7],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[7],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[7],4))) normalOn = true;
	}
	if (dateTime.hour == 15) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[7],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[7],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[7],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[7],0))) normalOn = true;
	}
	if (dateTime.hour == 16) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[8],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[8],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[8],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[8],4))) normalOn = true;
	}
	if (dateTime.hour == 17) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[8],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[8],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[8],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[8],0))) normalOn = true;
	}
	if (dateTime.hour == 18) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[9],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[9],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[9],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[9],4))) normalOn = true;
	}
	if (dateTime.hour == 19) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[9],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[9],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[9],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[9],0))) normalOn = true;
	}
	if (dateTime.hour == 20) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[10],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[10],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[10],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[10],4))) normalOn = true;
	}
	if (dateTime.hour == 21) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[10],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[10],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[10],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[10],0))) normalOn = true;
	}
	if (dateTime.hour == 22) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[11],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[11],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[11],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[11],4))) normalOn = true;
	}
	if (dateTime.hour == 23) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[11],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[11],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[11],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[11],0))) normalOn = true;
	}
	return normalOn;
}

void outputs() {
	if (device.bypassOpen) servo.write(0);
	else servo.write(90);
}

void readSensors() {
	if (sleep(&readSensorMillis, 5)) return;
	for (int i=0; i<4; i++) {
		device.sensorsBME280[i].temperature = device.sensorsBME280[i].interface.readTemperature();
		device.sensorsBME280[i].pressure = (int)(device.sensorsBME280[i].interface.readPressure()/100);
		device.sensorsBME280[i].humidity = (int)device.sensorsBME280[i].interface.readHumidity();
		if (device.sensorsBME280[i].temperature>35
				|| device.sensorsBME280[i].temperature<10
				|| device.sensorsBME280[i].pressure>1050
				|| device.sensorsBME280[i].pressure<800
				|| device.sensorsBME280[i].humidity>100
				|| device.sensorsBME280[i].humidity<15)
			device.sensorsBME280[i].faultyReadings++;
	}
}

void statusUpdate() {
	String status;
	status = "PARAMETRY PRACY\n";
	status +="Wentylator: "; status += device.fan ? "TAK":"NIE"; status +="\tNormalON: "; status += device.normalON ? "TAK":"NIE";
	status +="\tHumidityALERT: "; status += device.humidityAlert ? "TAK":"NIE"; status +="\n";
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

void getMasterDeviceOrder() {
	if ((UDPdata.data[0] >= 4)
			&& (UDPdata.data[0] <=15)) {
		int hour = UDPdata.data[0]-4;
		device.hour[hour] = UDPdata.data[1];
		EEpromWrite(hour, UDPdata.data[1]);
	}
	setUDPdata();
	forceStandardUDP();
}

void readUDPdata() {
	UDPdata = getDataRead();
	if (!UDPdata.newData) return;
	if ((UDPdata.deviceType == 1)
			&& (UDPdata.deviceNo == getModuleType())
			&& (UDPdata.frameNo == getModuleNo()))
		getMasterDeviceOrder();
	resetNewData();
}

void setUDPdata() {
	byte dataWrite[13];
	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = (device.fan << 7);
	dataWrite[1] = device.hour[0];
	dataWrite[2] = device.hour[1];
	dataWrite[3] = device.hour[2];
	dataWrite[4] = device.hour[3];
	dataWrite[5] = device.hour[4];
	dataWrite[6] = device.hour[5];
	dataWrite[7] = device.hour[6];
	dataWrite[8] = device.hour[7];
	dataWrite[9] = device.hour[8];
	dataWrite[10] = device.hour[9];
	dataWrite[11] = device.hour[10];
	dataWrite[12] = device.hour[11];

	setUDPdata(0, dataWrite,13);
}



