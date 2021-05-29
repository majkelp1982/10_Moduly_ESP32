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

	device.lights[0].forceMax = (EEpromData[0]>0)?true: false;
	device.lights[1].forceMax = (EEpromData[1]>0)?true: false;
	device.lights[2].forceMax = (EEpromData[2]>0)?true: false;
	device.lights[3].forceMax = (EEpromData[3]>0)?true: false;

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

	device.lights[0].maxIntens = EEpromData[15];
	device.lights[1].maxIntens = EEpromData[16];
	device.lights[2].maxIntens = EEpromData[17];
	device.lights[3].maxIntens = EEpromData[18];
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

void checkIfForceValid() {
	for (int i=0; i<4; i++) {
		if (device.lights[i].forceMax)
			device.lights[i].force0 = false;
		if (device.lights[i].force0)
			device.lights[i].forceMax = false;
	}
}

void getMasterDeviceOrder() {
	String message ="getMasterDeviceOrder data0=";
	message +=UDPdata.data[0];
	message +=" data1=";
	message +=UDPdata.data[1];
	debug(message);
	if (UDPdata.data[0] == 3) {
		device.lights[0].forceMax = (UDPbitStatus(UDPdata.data[1],7)>0)?true:false;
		device.lights[1].forceMax = (UDPbitStatus(UDPdata.data[1],6)>0)?true:false;
		device.lights[2].forceMax = (UDPbitStatus(UDPdata.data[1],5)>0)?true:false;
		device.lights[3].forceMax = (UDPbitStatus(UDPdata.data[1],4)>0)?true:false;
		device.lights[0].force0 = (UDPbitStatus(UDPdata.data[1],3)>0)?true:false;
		device.lights[1].force0 = (UDPbitStatus(UDPdata.data[1],2)>0)?true:false;
		device.lights[2].force0 = (UDPbitStatus(UDPdata.data[1],1)>0)?true:false;
		device.lights[3].force0 = (UDPbitStatus(UDPdata.data[1],0)>0)?true:false;
		checkIfForceValid();
		EEpromWrite(0, device.lights[0].forceMax);
		EEpromWrite(1, device.lights[1].forceMax);
		EEpromWrite(2, device.lights[2].forceMax);
		EEpromWrite(3, device.lights[3].forceMax);
		EEpromWrite(4, device.lights[0].force0);
		EEpromWrite(5, device.lights[1].force0);
		EEpromWrite(6, device.lights[2].force0);
		EEpromWrite(7, device.lights[3].force0);
	}
	if (UDPdata.data[0] == 8) {
		device.startLightLevel= UDPdata.data[1];
		EEpromWrite(8, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 9) {
		device.lights[0].standByIntens = UDPdata.data[1];
		EEpromWrite(9, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 10) {
		device.lights[1].standByIntens = UDPdata.data[1];
		EEpromWrite(10, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 11) {
		device.lights[2].standByIntens = UDPdata.data[1];
		EEpromWrite(11, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 12) {
		device.lights[3].standByIntens = UDPdata.data[1];
		EEpromWrite(12, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 13) {
		device.offTime.hour = UDPdata.data[1];
		EEpromWrite(13, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 14) {
		device.offTime.minute = UDPdata.data[1];
		EEpromWrite(14, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 15) {
		device.lights[0].maxIntens = UDPdata.data[1];
		EEpromWrite(15, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 16) {
		device.lights[1].maxIntens = UDPdata.data[1];
		EEpromWrite(16, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 17) {
		device.lights[2].maxIntens = UDPdata.data[1];
		EEpromWrite(17, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 18) {
		device.lights[3].maxIntens = UDPdata.data[1];
		EEpromWrite(18, UDPdata.data[1]);
	}}

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
		intens=100;
	if (intens<0)
		intens=0;
	return intens;
}

void intensCheck(int id) {
	if (device.lights[id].expIntens == device.lights[id].isIntens)
		return;
	if (millis()<(device.lights[id].lastCorrection+device.lights[id].delay))
		return;
	device.lights[id].lastCorrection = millis();
	(device.lights[id].expIntens>device.lights[id].isIntens)? device.lights[id].isIntens++:device.lights[id].isIntens--;
}

void dimmerProcessing(int id) {
	device.lights[id].expIntens = calculateIntens(device.lights[id].standByIntens);
	if (device.nightTime) {
		device.lights[id].expIntens = 0;
	}
	if (device.lights[id].forceMax)
		device.lights[id].expIntens = 100;
	if (device.lights[id].force0)
		device.lights[id].expIntens = 0;

	if (device.lights[id].expIntens>device.lights[id].maxIntens)
		device.lights[id].expIntens = device.lights[id].maxIntens;
	intensCheck(id);
	device.lights[id].interface.setPower(device.lights[id].isIntens);
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

	dimmerProcessing(ID_ENTRANCE);
	dimmerProcessing(ID_DRIVEWAY);
	dimmerProcessing(ID_CARPORT);
	dimmerProcessing(ID_FENCE);
}

void outputs() {
}

void setUDPdata() {
	int size = 16;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = 0;
	dataWrite[0] = (device.lights[0].forceMax << 7 | device.lights[1].forceMax << 6 | device.lights[2].forceMax << 5 | device.lights[3].forceMax << 4 |
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
	dataWrite[12] = (byte)(device.lights[0].maxIntens);
	dataWrite[13] = (byte)(device.lights[1].maxIntens);
	dataWrite[14] = (byte)(device.lights[2].maxIntens);
	dataWrite[15] = (byte)(device.lights[3].maxIntens);

	setUDPdata(0, dataWrite,size);
}

String lightStatus(Light light, String name) {
	String status;
	status +="\nLight["; status +=name; status +="]\t\tforceMax="; status+=light.forceMax; status +="\tforce0="; status+=light.force0;
	status +="\tisIntens=="; status+=light.isIntens; status +="%\texpIntens=="; status+=light.expIntens;
	status +="%\tstandByIntens=="; status+=light.standByIntens; status +="%\tmaxIntens=="; status+=light.maxIntens;  status +="%\tdelay="; status+=light.delay;
	return status;
}

void statusUpdate() {
	String status;
	status = "PARAMETRY \n";
	status += lightStatus(device.lights[ID_ENTRANCE], "ENTRANCE");
	status += lightStatus(device.lights[ID_DRIVEWAY], "DRIVEWAY");
	status += lightStatus(device.lights[ID_CARPORT], "CARPORT");
	status += lightStatus(device.lights[ID_FENCE], "FENCE");

	status +="\n\nPróg w³¹czenia="; status +=device.startLightLevel; status+="[%]";
	status +="\n\nGodzina wy³¹czenia="; status +=device.offTime.hour; status+=":"; status +=device.offTime.minute;
	status +="\n\Wy³¹czenie nocne ="; device.nightTime?status +="TAK": status +="NIE";
	status +="\n\nIntensynoœ œwiat³a [modul_pogodowy]="; status +=device.lightSensor; status+="[%]";

	setStatus(status);
}
