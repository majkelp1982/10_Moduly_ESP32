#include "Module.h"

Device device;
DataRead UDPdata;

//Functions
void readSensors();
void readBME280();
void readSDS011();
void readLightIntens();
void readUDPdata();
void getMasterDeviceOrder();

void setUDPdata();
void statusUpdate();

//Variables
Adafruit_BME280 bme1(CS_BME280, SPI_MOSI, SPI_MISO, SPI_SCK);
SDS011 sds011(SERIAL2_RX, SERIAL2_TX);

//Delays
unsigned long readSensorMillis = 0;
unsigned long readLightSensorMillis = 0;

int light;
//TMP
int lastDutyCycle = 0;

void module_init() {
	//Light sensor
	pinMode(PIN_LIGHT, INPUT);

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
	readLightIntens();
}

void readBME280() {
	if (sleep(&readSensorMillis, 5)) return;
	float temperature = device.sensorBME280.interface.readTemperature();
	int pressure = (int)(device.sensorBME280.interface.readPressure()/100);
	int humidity = (int)device.sensorBME280.interface.readHumidity();
	if (temperature>70
			|| pressure>1050
			|| pressure<800
			|| humidity>100
			|| humidity<15)
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
				// if after 4 seconds of stable work sensor still doesn't work properly add  error and switch to sleep
				if (modeSwitchTime>(device.sensorSDS011.standUpTime+4)) {
					device.sensorSDS011.faultyReadings++;
				}
			}
	}
}

void readLightIntens() {
	if (sleep(&readLightSensorMillis, 5)) return;
	light = analogRead(PIN_LIGHT);
	device.lightSensor = (int)(light*100/4095);

	//Fault strategy
	//fixme
	int hour = getDateTime().hour;
	if ((hour >=17) || (hour <5))
		device.lightSensor = 0;
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
	int size = 10;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	//BMEs
	dataWrite[0] = get10Temp(device.sensorBME280.temperature);
	dataWrite[1] = get01Temp(device.sensorBME280.temperature);
	dataWrite[2] = device.sensorBME280.humidity;
	dataWrite[3] = (byte)(device.sensorBME280.pressure>>8);
	dataWrite[4] = (byte)(device.sensorBME280.pressure);
	dataWrite[5] = (byte)(device.sensorSDS011.pm25>>8);
	dataWrite[6] = (byte)(device.sensorSDS011.pm25);
	dataWrite[7] = (byte)(device.sensorSDS011.pm10>>8);
	dataWrite[8] = (byte)(device.sensorSDS011.pm10);
	dataWrite[9] = (byte)(device.lightSensor);

	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status = "PARAMETRY \n";
	status +="BME280\t\tT="; status +=device.sensorBME280.temperature; status +="[stC]\t\tH="; status +=(int)device.sensorBME280.humidity;
	status +="[%]\t\tP="; status +=(int)device.sensorBME280.pressure;status +="[hPa]\t\tFaulty="; status +=(int)device.sensorBME280.faultyReadings ;status +="\n";
	status +="SDS011\t\tMode="; status+= (device.sensorSDS011.interface.mode==MODE_SLEEP)? "SLEEP":"WAKEUP"; status +="\t\tTimeLeft="; status +=device.sensorSDS011.modeTimeLeft; status+="[s]\n";
	status +="\t\tPM2.5="; status +=device.sensorSDS011.pm25; status +="[ug/m3]\t\tPM10="; status +=(int)device.sensorSDS011.pm10; status +="[ug/m3]\t\tFaulty="; status +=(int)device.sensorSDS011.faultyReadings ;status +="\n";
	status +="\nLightSensor="; status += device.lightSensor; status +="[%]"; status +=" AnalogVal="; status += light;

	setStatus(status);
}
