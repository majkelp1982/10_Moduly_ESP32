#include "Module.h"

Device device;
DataRead UDPdata;

//Functions
void firstScan();
void pinDef();
void readSensor();
void readUDPdata();
void getMasterDeviceOrder();

//Module specific functions

void pumps();

//Output functions
void setUDPdata();
void statusUpdate();
void outputs();


//Delays
unsigned long sensorReadMillis = 0;


void module_init() {
	//EEprom Scan
	firstScan();

	//pin definitions
	pinDef();
}

void firstScan() {
	// Bajty EEPROM //
	// 0 - 07		:   18b20 device address inlet
	// 08 - 15		:   18b20 device address outlet
	// 16 - 23		:   18b20 device address chimney
	// 24			:   temperature set in C

	int size = 1024;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");

	device.maxWaterLevel = EEpromData[0];
	device.minWaterLevel = EEpromData[1];
	device.airInterval = EEpromData[2];

}

void pinDef() {
	pinMode (pinAIR_PUMP,OUTPUT);	digitalWrite(pinAIR_PUMP, LOW);
	pinMode (pinWATER_PUMP,OUTPUT);	digitalWrite(pinWATER_PUMP, LOW);
}

void module() {
	//Input data
	readSensor();
	readUDPdata();

	//Main functions
	pumps();

	//Output settings
	setUDPdata();
	statusUpdate();
	outputs();
}

void readSensor() {
	if (sleep(&sensorReadMillis, DELAY_SENSOR_READ)) return;
	//TODO
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
	//TODO
//	if (UDPdata.data[0] == 2) {
//		device.reqTemp = UDPdata.data[1];
//		EEpromWrite(24, UDPdata.data[1]);
//	}
//
//	//TMP
//	if (UDPdata.data[0] == 3)
//		device.thermo[0].isTemp = UDPdata.data[1];
//
//	if (UDPdata.data[0] == 4)
//		device.thermo[1].isTemp = UDPdata.data[1];
//
//	if (UDPdata.data[0] == 5)
//		device.thermo[2].isTemp = UDPdata.data[1];
}

void pumps() {
	//TODO
//	device.pump = false;
//	if (device.mode == 2)
//		device.pump = true;
//
//	if (device.mode == 3)
//		device.pump = true;
//
//	if (screen.info[0].active)
//		device.pump = true;
//
//	if (screen.info[1].active)
//		device.pump = true;
}

void setUDPdata() {
	int size = 5;
	byte dataWrite[size];
	dataWrite[0] = (device.airPump << 7) | (device.waterPump << 6);
	dataWrite[1] = device.isWaterLevel;
	dataWrite[2] = (byte)device.maxWaterLevel;
	dataWrite[3] = (byte)device.minWaterLevel;
	dataWrite[4] = (byte)device.airInterval;
	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	//TODO
//	status +="STAN";
//	if (device.mode == 0) status += "\tCZUWANIE";
//	if (device.mode == 1) status += "\tROZPALANIE";
//	if (device.mode == 2) status += "\tGRZANIE";
//	if (device.mode == 3) status += "\tWYGASZANIE";
//	status += "\tPompa["; status += device.pump; status+="]";
//	status += "\tPrzepustnica["; status += device.throttle; status+="]";
//	status += "\tStartTemp["; status += device.startTemp; status+="]";
//	status += "\nAlarm["; status += device.alarm; status+="]";
//	status += "\tWarning["; status += device.warning; status+="]";
//	status += "\tFireAlarm["; status += device.fireAlarm; status+="]";
//	status += "\tLowestTemp["; status += device.lowestTemp; status+="]";
//	status += "\nPARAMETRY";
//	status +="\nTemp.ustawiona\tT="; status +=device.reqTemp; status +="[stC]";
//	status +="\nTemp.WE\t\tT="; status +=device.thermo[ID_INLET].isTemp; status +="[stC]";
//	status +="\nTemp.WY\t\tT="; status +=device.thermo[ID_OUTLET].isTemp; status +="[stC]";
//	status +="\nTemp.KOMIN\tT="; status +=device.thermo[ID_CHIMNEY].isTemp; status +="[stC]\n";
	setStatus(status);
}

void outputs() {
	//TODO
}

