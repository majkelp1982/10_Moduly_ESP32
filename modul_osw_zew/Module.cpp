#include "Module.h"

Device device;
DataRead UDPdata;

dimmerLamp dimmer0(pinLIGHT0, pinZERO_CROSSING);
dimmerLamp dimmer1(pinLIGHT1, pinZERO_CROSSING);
dimmerLamp dimmer2(pinLIGHT2, pinZERO_CROSSING);
dimmerLamp dimmer3(pinLIGHT3, pinZERO_CROSSING);

//Delays
unsigned long sensorReadMillis = 0;

//Functions
void firstScan();
void pinDef();
void readSensor();
void readUDPdata();
void getMasterDeviceOrder();

void dimmers();
void outputs();

void setUDPdata();
void statusUpdate();


void module_init() {
	//Dimmer declaration
	device.lights[ID_ENTRANCE].dimmer = dimmer0;
	device.lights[ID_DRIVEWAY].dimmer = dimmer1;
	device.lights[ID_CARPORT].dimmer = dimmer2;
	device.lights[ID_FENCE].dimmer = dimmer3;

	device.lights[ID_ENTRANCE].dimmer.begin(NORMAL_MODE, ON);
	device.lights[ID_DRIVEWAY].dimmer.begin(NORMAL_MODE, ON);
	device.lights[ID_CARPORT].dimmer.begin(NORMAL_MODE, ON);
	device.lights[ID_FENCE].dimmer.begin(NORMAL_MODE, ON);

	//EEprom scan
	firstScan();
	//pin definitions
	pinDef();
}

void firstScan() {
	// Bajty EEPROM //
	int size = 1024;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");

	device.turnOnLightLevel = EEpromData[0];
	device.standByIntensLevel = EEpromData[1];
	device.offTime.hour = EEpromData[2];
	device.offTime.minute = EEpromData[3];
}

void pinDef() {
	//Light sensor
	pinMode (pinSENSOR,INPUT_PULLUP);
}

void module() {
	//Input data
	readSensor();
	readUDPdata();

	//Main
	dimmers();

	//Output settings
	setUDPdata();
	statusUpdate();
}

void readSensor() {
	if (sleep(&sensorReadMillis, DELAY_SENSOR_READ)) return;
	int value = analogRead(pinSENSOR);
	device.lightSensor = (byte)((100/255) * value);
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
	String message ="getMasterDeviceOrder data0=";
	message +=UDPdata.data[0];
	message +=" data1=";
	message +=UDPdata.data[1];
	debug(message);
	if (UDPdata.data[0] == 5) {
		device.turnOnLightLevel = UDPdata.data[1];
		EEpromWrite(0, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 1) {
		device.standByIntensLevel = UDPdata.data[1];
		EEpromWrite(1, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 2) {
		device.offTime.hour = UDPdata.data[1];
		EEpromWrite(2, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 3) {
		device.offTime.minute = UDPdata.data[1];
		EEpromWrite(3, UDPdata.data[1]);
	}
}

void dimmers() {
	//intens = (100-sensor)-próg*100/próg
	byte intens = 0;
	if (device.turnOnLightLevel != 0)
		intens = (100-device.lightSensor-device.turnOnLightLevel)*100/device.turnOnLightLevel;
	else
		if (device.lightSensor == 0)
			intens = 100;

	if (intens>=100)
		intens=99;

	device.lights[ID_ENTRANCE].dimmer.setPower(intens);
	device.lights[ID_DRIVEWAY].dimmer.setPower(intens);
	device.lights[ID_CARPORT].dimmer.setPower(intens);
	device.lights[ID_FENCE].dimmer.setPower(intens);
}

void outputs() {

}

void setUDPdata() {
	int size = 9;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	//BMEs
	dataWrite[0] = device.lightSensor;
	dataWrite[1] = (byte)device.lights[ID_ENTRANCE].dimmer.getPower();
	dataWrite[2] = (byte)device.lights[ID_DRIVEWAY].dimmer.getPower();
	dataWrite[3] = (byte)device.lights[ID_CARPORT].dimmer.getPower();
	dataWrite[4] = (byte)device.lights[ID_FENCE].dimmer.getPower();
	dataWrite[5] = (byte)(device.turnOnLightLevel);
	dataWrite[6] = (byte)(device.standByIntensLevel);
	dataWrite[7] = (byte)(device.offTime.hour);
	dataWrite[8] = (byte)(device.offTime.minute);

	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status = "PARAMETRY \n";
	status +="\nSensor œwiat³a="; status +=device.lightSensor; status +="[%]";
	status +="\nLight[WEJSCIE]="; status +=device.lights[ID_ENTRANCE].dimmer.getPower(); status +="[%]";
	status +="\nLight[PODJAZD]="; status +=device.lights[ID_DRIVEWAY].dimmer.getPower(); status +="[%]";
	status +="\nLight[CARPORT]="; status +=device.lights[ID_CARPORT].dimmer.getPower(); status +="[%]";
	status +="\nLight[OGRODZENIE]="; status +=device.lights[ID_FENCE].dimmer.getPower(); status +="[%]";
	status +="\n\nPróg w³¹czenia="; status +=device.turnOnLightLevel; status+="[%]";
	status +="\nPróg intensywnoœci="; status +=device.standByIntensLevel; status+="[%]";
	status +="\n\nGodzina wy³¹czenia="; status +=device.offTime.hour; status+=":"; status +=device.offTime.minute;

	setStatus(status);
}
