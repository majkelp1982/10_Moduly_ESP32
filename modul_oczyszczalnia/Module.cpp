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
	device.zeroReference = EEpromData[3];

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
	digitalWrite(pinTRIG, LOW);
	delayMicroseconds(2);
	digitalWrite(pinTRIG, HIGH);
	delayMicroseconds(15);
	digitalWrite(pinTRIG, LOW);
	digitalWrite(pinECHO, HIGH);
	long time = pulseIn(pinECHO, HIGH);
	device.isWaterLevel = (int)(time / 58);
}

void readUDPdata() {
	UDPdata = getDataRead();
	if (!UDPdata.newData) return;
	debug("readUDPdata-new data");
	if ((UDPdata.deviceType == ID_MOD_MAIN)
			&& (UDPdata.deviceNo == getModuleType())
			&& (UDPdata.frameNo == getModuleNo()))
		getMasterDeviceOrder();
	resetNewData();
}

void getMasterDeviceOrder() {
	String message ="getMasterDeviceOrder data0="+UDPdata.data[0];
	message.concat(" data1=");
	message.concat(UDPdata.data[1]);
	debug(message);
	if (UDPdata.data[0] == 5) {
		device.maxWaterLevel = UDPdata.data[1];
		EEpromWrite(0, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 6) {
		device.minWaterLevel = UDPdata.data[1];
		EEpromWrite(1, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 7) {
		device.airInterval= UDPdata.data[1];
		EEpromWrite(2, UDPdata.data[1]);
	}
	if (UDPdata.data[0] == 8) {
		device.zeroReference = UDPdata.data[1];
		EEpromWrite(3, UDPdata.data[1]);
	}
}

void pumps() {
	//Air Pump
	unsigned long lastStateChange = millis() - device.lastStateChange;
	if (device.airInterval<1)
		device.airInterval = 1;
	if ((int)(lastStateChange/60000)>=device.airInterval) {
		device.lastStateChange = millis();
		device.airPump = !device.airPump;
	}

	//Water Pump
	if (device.isWaterLevel>=device.maxWaterLevel)
		device.waterPump = true;
	if (device.isWaterLevel<=device.minWaterLevel)
		device.waterPump = false;
}

void setUDPdata() {
	int size = 6;
	byte dataWrite[size];
	dataWrite[0] = (device.airPump << 7) | (device.waterPump << 6);
	dataWrite[1] = device.isWaterLevel;
	dataWrite[2] = (byte)device.maxWaterLevel;
	dataWrite[3] = (byte)device.minWaterLevel;
	dataWrite[4] = (byte)device.airInterval;
	dataWrite[5] = (byte)device.zeroReference;
	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status +="POMPY";
	status += "\tAirPump["; status += device.airPump; status+="]";
	status += "\tWaterPump["; status += device.waterPump; status+="]";
	status +="\nWODA";
	status += "\tisLevel[-"; status += device.isWaterLevel; status+="]cm";
	status += "\tmaxLevel[-"; status += device.maxWaterLevel; status+="]cm";
	status += "\tminLevel[-"; status += device.minWaterLevel; status+="]cm";
	status += "\tzeroReference[-"; status += device.zeroReference; status+="]cm";
	status +="\nNAPOWIETRZANIE";
	status += "\tinterwal["; status += device.airInterval; status+="]min";
	status += "\tlastStateChange["; status += (int)((millis() - device.lastStateChange)/1000); status+="]s";
	setStatus(status);
}

void outputs() {
	digitalWrite(pinAIR_PUMP, !device.airPump);
	digitalWrite(pinWATER_PUMP, !device.waterPump);
}

