#include "Module.h"

Device device;
DataRead UDPdata;

//Functions
void readSensors();
void readBME280();
void readSDS011();
void readUDPdata();
void getMasterDeviceOrder();

void setUDPdata();
void statusUpdate();

//help functions
int get10Temp(float temp);
int get01Temp(float temp);

//Variables
Adafruit_BME280 bme1(CS_BME280, SPI_MOSI, SPI_MISO, SPI_SCK);
SDS011 sds011(SERIAL2_RX, SERIAL2_TX);

//Delays
unsigned long readSensorMillis = 0;

//TMP
int lastDutyCycle = 0;

void module_init() {
	//Set CS pins
	device.sensorBME280.interface= bme1;
	//initialization
	device.sensorBME280.interface.begin();

	device.sensorSDS011.interface = sds011;
	device.sensorSDS011.interface.begin();
	device.sensorSDS011.interface.wakeup();
}

void module() {
	//Input data
	readSensors();
	readUDPdata();

	//Output settings
	setUDPdata();
	statusUpdate();
}

void readSensors() {
	readBME280();
	readSDS011();
}

void readBME280() {
	if (sleep(&readSensorMillis, 5)) return;
	float temperature = device.sensorBME280.interface.readTemperature();
	int pressure = (int)(device.sensorBME280.interface.readPressure());
	int humidity = (int)device.sensorBME280.interface.readHumidity();
	if (device.sensorBME280.temperature>70
			|| device.sensorBME280.pressure>1050
			|| device.sensorBME280.pressure<800
			|| device.sensorBME280.humidity>100
			|| device.sensorBME280.humidity<15)
		device.sensorBME280.faultyReadings++;
	else {
		device.sensorBME280.temperature = temperature;
		device.sensorBME280.pressure = pressure;
		device.sensorBME280.humidity = humidity;
	}
}

void readSDS011() {
	unsigned long modeSwitchTime = (unsigned long)((millis()-device.sensorSDS011.interface.modeTime)/1000);			// how long module is in this mode in seconds
	if (device.sensorSDS011.interface.mode==MODE_SLEEP) {
		device.sensorSDS011.modeTimeLeft = device.sensorSDS011.sleepTime-modeSwitchTime;
		if (modeSwitchTime<device.sensorSDS011.sleepTime)
			return;
		else device.sensorSDS011.interface.wakeup();
	}
	float p25 = 0;
	float p10 = 0;
	if (device.sensorSDS011.interface.mode==MODE_WAKEUP) {
		device.sensorSDS011.modeTimeLeft = device.sensorSDS011.standUpTime-modeSwitchTime;
		if (modeSwitchTime<device.sensorSDS011.standUpTime)
			device.sensorSDS011.interface.read(&p25, &p10);
		else {
			int err = device.sensorSDS011.interface.read(&p25,&p10);
			if (err == 0) {
				device.sensorSDS011.pm25 = p25;
				device.sensorSDS011.pm10 = p10;
				device.sensorSDS011.interface.sleep();
			}
			else
				device.sensorSDS011.faultyReadings++;
		}
	}
}

void readUDPdata() {
	UDPdata = getDataRead();
	if (!UDPdata.newData) return;
	if ((UDPdata.deviceType == ID_MOD_MAIN)
			&& (UDPdata.deviceNo == getModuleType())
			&& (UDPdata.frameNo == getModuleNo()))
		getMasterDeviceOrder();

	resetNewData();
}

void getMasterDeviceOrder() {
}

void setUDPdata() {
	int size = 37;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	//BMEs
	dataWrite[0] = get10Temp(device.sensorBME280.temperature);
	dataWrite[1] = get01Temp(device.sensorBME280.temperature);
	dataWrite[2] = device.sensorBME280.humidity;
	dataWrite[3] = (int)(device.sensorBME280.pressure/10);
	dataWrite[4] = (int)(device.sensorBME280.pressure/10);
	dataWrite[5] = (int)(device.sensorBME280.pressure/10);

	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status = "PARAMETRY \n";
	status +="BME280\tT="; status +=device.sensorBME280.temperature; status +="[stC]\tH="; status +=(int)device.sensorBME280.humidity;
	status +="[%]\tP="; status +=(int)device.sensorBME280.pressure;status +="[hPa] Faulty="; status +=(int)device.sensorBME280.faultyReadings ;status +="\n";
	status +="SDS011\t Mode="; status+= (device.sensorSDS011.mode==MODE_SLEEP)? "SLEEP":"WAKEUP"; status +="\t TimeLeft="; status +=device.sensorSDS011.modeTimeLeft; status+="[s]\n";
	status +="PM2.5="; status +=device.sensorSDS011.pm25; status +="[ug/m3]\tPM10="; status +=(int)device.sensorSDS011.pm10; status +="[ug/m3]\t Faulty="; status +=(int)device.sensorSDS011.faultyReadings ;status +="\n";

	status +=device.sensorSDS011.interface.modeTime;
	status +="\n";
	status +=(unsigned long)(device.sensorSDS011.interface.modeTime/1000);
	setStatus(status);
}

//Help functions
int get10Temp(float temp){
	return (int)temp;
}

int get01Temp(float temp) {
	int temp10 = get10Temp(temp);
	return (int) ((temp*10)-(temp10*10));
}




