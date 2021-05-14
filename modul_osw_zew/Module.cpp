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
void readUDPdata();
void getMasterDeviceOrder();
void getWeatherParams();

void dimmers();
void outputs();

void setUDPdata();
void statusUpdate();


void module_init() {
	//Dimmer declaration
	device.lights[ID_ENTRANCE] = dimmer0;
	device.lights[ID_DRIVEWAY] = dimmer1;
	device.lights[ID_CARPORT] = dimmer2;
	device.lights[ID_FENCE] = dimmer3;

	device.lights[ID_ENTRANCE].begin(NORMAL_MODE, ON);
	device.lights[ID_DRIVEWAY].begin(NORMAL_MODE, ON);
	device.lights[ID_CARPORT].begin(NORMAL_MODE, ON);
	device.lights[ID_FENCE].begin(NORMAL_MODE, ON);

	//EEprom scan
	firstScan();
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
}

void module() {
	//Input data
	readUDPdata();

	//Main
	dimmers();

	//Output settings
	setUDPdata();
	statusUpdate();
}

void readUDPdata() {
	UDPdata = getDataRead();
	if (!UDPdata.newData) return;
	if ((UDPdata.deviceType == ID_MOD_MAIN)
			&& (UDPdata.deviceNo == getModuleType())
			&& (UDPdata.frameNo == getModuleNo()))
		getMasterDeviceOrder();
	if ((UDPdata.deviceType == ID_MOD_WEATHER)
			&& (UDPdata.deviceNo == 0)
			&& (UDPdata.frameNo == 0))
		getWeatherParams();
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
	if (UDPdata.data[0] == 6) {
		device.standByIntensLevel = UDPdata.data[1];
		EEpromWrite(1, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 7) {
		device.offTime.hour = UDPdata.data[1];
		EEpromWrite(2, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 8) {
		device.offTime.minute = UDPdata.data[1];
		EEpromWrite(3, UDPdata.data[1]);
	}
}

void getWeatherParams() {
	device.lightSensor = UDPdata.data[9];
}

void dimmers() {
	//intens = (100-sensor)-próg*100/próg
	int intens = 0;
	if ((device.turnOnLightLevel != 0) && (device.standByIntensLevel != 0))
		intens = ((device.turnOnLightLevel-device.lightSensor)*device.standByIntensLevel)/(device.turnOnLightLevel);
	else
		if (device.lightSensor <= 10)
			intens = 100;

	if (intens>=100)
		intens=99;
	if (intens<0)
		intens=0;

	device.lights[ID_ENTRANCE].setPower(intens);
	device.lights[ID_DRIVEWAY].setPower(intens);
	device.lights[ID_CARPORT].setPower(intens);
	device.lights[ID_FENCE].setPower(intens);
}

void outputs() {

}

void setUDPdata() {
	int size = 9;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = 0;
	dataWrite[1] = (byte)device.lights[ID_ENTRANCE].getPower();
	dataWrite[2] = (byte)device.lights[ID_DRIVEWAY].getPower();
	dataWrite[3] = (byte)device.lights[ID_CARPORT].getPower();
	dataWrite[4] = (byte)device.lights[ID_FENCE].getPower();
	dataWrite[5] = (byte)(device.turnOnLightLevel);
	dataWrite[6] = (byte)(device.standByIntensLevel);
	dataWrite[7] = (byte)(device.offTime.hour);
	dataWrite[8] = (byte)(device.offTime.minute);

	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status = "PARAMETRY \n";
	status +="\nLight[WEJSCIE]="; status +=device.lights[ID_ENTRANCE].getPower(); status +="[%]";
	status +="\nLight[PODJAZD]="; status +=device.lights[ID_DRIVEWAY].getPower(); status +="[%]";
	status +="\nLight[CARPORT]="; status +=device.lights[ID_CARPORT].getPower(); status +="[%]";
	status +="\nLight[OGRODZENIE]="; status +=device.lights[ID_FENCE].getPower(); status +="[%]";
	status +="\n\nPróg w³¹czenia="; status +=device.turnOnLightLevel; status+="[%]";
	status +="\nPróg intensywnoœci="; status +=device.standByIntensLevel; status+="[%]";
	status +="\n\nGodzina wy³¹czenia="; status +=device.offTime.hour; status+=":"; status +=device.offTime.minute;
	status +="\n\nIntensynoœ œwiat³a [modul_pogodowy]="; status +=device.lightSensor; status+="[%]";

	setStatus(status);
}
