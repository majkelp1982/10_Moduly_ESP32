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
	device.lights[ID_ENTRANCE].interface = dimmer0;
	device.lights[ID_DRIVEWAY].interface = dimmer1;
	device.lights[ID_CARPORT].interface = dimmer2;
	device.lights[ID_FENCE].interface = dimmer3;

	device.lights[ID_ENTRANCE].interface.begin(NORMAL_MODE, ON);
	device.lights[ID_DRIVEWAY].interface.begin(NORMAL_MODE, ON);
	device.lights[ID_CARPORT].interface.begin(NORMAL_MODE, ON);
	device.lights[ID_FENCE].interface.begin(NORMAL_MODE, ON);

	//EEprom scan
	firstScan();
}

void firstScan() {
	// Bajty EEPROM //
	int size = 1024;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");

	device.lights[0].force100 = (EEpromData[0]>0)?true: false;
	device.lights[1].force100 = (EEpromData[1]>0)?true: false;
	device.lights[2].force100 = (EEpromData[2]>0)?true: false;
	device.lights[3].force100 = (EEpromData[3]>0)?true: false;

	device.lights[0].force0 = (EEpromData[4]>0)?true: false;
	device.lights[1].force0 = (EEpromData[5]>0)?true: false;
	device.lights[2].force0 = (EEpromData[6]>0)?true: false;
	device.lights[3].force0 = (EEpromData[7]>0)?true: false;

	device.startLightLevel= EEpromData[8];
	device.lights[0].standByIntens = EEpromData[9];
	device.lights[1].standByIntens = EEpromData[10];
	device.lights[2].standByIntens = EEpromData[11];
	device.lights[3].standByIntens = EEpromData[12];

	device.offTime.hour = EEpromData[13];
	device.offTime.minute = EEpromData[14];
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
	if (UDPdata.data[0] == 0) {
		device.lights[0].force100 = (UDPbitStatus(UDPdata.data[1],7)>0)?true:false;
		device.lights[1].force100 = (UDPbitStatus(UDPdata.data[1],6)>0)?true:false;
		device.lights[2].force100 = (UDPbitStatus(UDPdata.data[1],5)>0)?true:false;
		device.lights[3].force100 = (UDPbitStatus(UDPdata.data[1],4)>0)?true:false;
		device.lights[0].force0 = (UDPbitStatus(UDPdata.data[1],3)>0)?true:false;
		device.lights[1].force0 = (UDPbitStatus(UDPdata.data[1],2)>0)?true:false;
		device.lights[2].force0 = (UDPbitStatus(UDPdata.data[1],1)>0)?true:false;
		device.lights[3].force0 = (UDPbitStatus(UDPdata.data[1],0)>0)?true:false;
		EEpromWrite(0, device.lights[0].force100);
		EEpromWrite(1, device.lights[1].force100);
		EEpromWrite(2, device.lights[2].force100);
		EEpromWrite(3, device.lights[3].force100);
		EEpromWrite(4, device.lights[0].force0);
		EEpromWrite(5, device.lights[1].force0);
		EEpromWrite(6, device.lights[2].force0);
		EEpromWrite(7, device.lights[3].force0);
	}
	if (UDPdata.data[0] == 5) {
		device.startLightLevel= UDPdata.data[1];
		EEpromWrite(8, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 6) {
		device.lights[0].standByIntens = UDPdata.data[1];
		EEpromWrite(9, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 7) {
		device.lights[1].standByIntens = UDPdata.data[1];
		EEpromWrite(10, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 8) {
		device.lights[2].standByIntens = UDPdata.data[1];
		EEpromWrite(11, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 9) {
		device.lights[3].standByIntens = UDPdata.data[1];
		EEpromWrite(12, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 10) {
		device.offTime.hour = UDPdata.data[1];
		EEpromWrite(13, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 11) {
		device.offTime.minute = UDPdata.data[1];
		EEpromWrite(14, UDPdata.data[1]);
	}
}

void getWeatherParams() {
	device.lightSensor = UDPdata.data[9];
}

int calculateIntens(int standByIntensLevel) {
	//intens = (100-sensor)-próg*100/próg
	int intens = 0;
	if ((device.startLightLevel!= 0) && (standByIntensLevel != 0))
		intens = ((device.startLightLevel-device.lightSensor)*standByIntensLevel)/(device.startLightLevel);
	else
		if (device.lightSensor <= 10)
			intens = 100;

	if (intens>=100)
		intens=99;
	if (intens<0)
		intens=0;
	return intens;
}

void intensCheck(Light light) {
	if (light.expIntens == light.isIntens)
		return;
	if (millis()<(light.lastCorrection+light.delay))
		return;
	(light.expIntens>light.isIntens)? light.isIntens++:light.isIntens--;
}

void dimmerProcessing(Light light) {
	light.expIntens = calculateIntens(light.standByIntens);
	if (device.nightTime)
		light.expIntens = 0;
	intensCheck(light);
	light.interface.setPower(light.isIntens);
}

void dimmers() {
	// Night time reset
	if (getDateTime().hour==5)
		device.nightTime = false;

	if ((getDateTime().hour== device.offTime.hour)
			&& (getDateTime().minute>=device.offTime.minute))
		device.nightTime = true;

	//force night time off
	if (device.offTime.hour==12)
		device.nightTime = false;

	dimmerProcessing(device.lights[ID_ENTRANCE]);
	dimmerProcessing(device.lights[ID_DRIVEWAY]);
	dimmerProcessing(device.lights[ID_CARPORT]);
	dimmerProcessing(device.lights[ID_FENCE]);
}

void outputs() {
}

void setUDPdata() {
	int size = 9;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = 0;
	dataWrite[0] = (device.lights[0].force100 << 7 | device.lights[1].force100 << 6 | device.lights[2].force100 << 5 | device.lights[3].force100 << 4 |
			device.lights[0].force0 << 3 | device.lights[1].force0 << 2 | device.lights[2].force0 << 1 | device.lights[3].force0 << 0);
	dataWrite[1] = (byte)device.lights[ID_ENTRANCE].isIntens;
	dataWrite[2] = (byte)device.lights[ID_DRIVEWAY].isIntens;
	dataWrite[3] = (byte)device.lights[ID_CARPORT].isIntens;
	dataWrite[4] = (byte)device.lights[ID_FENCE].isIntens;
	dataWrite[5] = (byte)(device.startLightLevel);
	dataWrite[6] = (byte)(device.lights[0].standByIntens);
	dataWrite[7] = (byte)(device.lights[1].standByIntens);
	dataWrite[8] = (byte)(device.lights[2].standByIntens);
	dataWrite[9] = (byte)(device.lights[3].standByIntens);
	dataWrite[10] = (byte)(device.offTime.hour);
	dataWrite[11] = (byte)(device.offTime.minute);

	setUDPdata(0, dataWrite,size);
}

String lightStatus(Light light, String name) {
	String status;
	status +="\nLight["; status +=name; status +="] force100="; status+=light.force100; status +=" force0="; status+=light.force0;
	status +=" isIntens=="; status+=light.isIntens; status +="% expIntens=="; status+=light.expIntens;
	status +="% standByIntens=="; status+=light.standByIntens; status +="% delay="; status+=light.delay;
	return status;
}

void statusUpdate() {
	String status;
	status = "PARAMETRY \n";
	status += lightStatus(device.lights[0], "ENTRANCE");
	status += lightStatus(device.lights[0], "DRIVEWAY");
	status += lightStatus(device.lights[0], "CARPORT");
	status += lightStatus(device.lights[0], "FENCE");

	status +="\n\nPróg w³¹czenia="; status +=device.startLightLevel; status+="[%]";
	status +="\n\nGodzina wy³¹czenia="; status +=device.offTime.hour; status+=":"; status +=device.offTime.minute;
	status +="\n\nIntensynoœ œwiat³a [modul_pogodowy]="; status +=device.lightSensor; status+="[%]";

	setStatus(status);
}
