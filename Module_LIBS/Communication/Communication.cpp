/*
 * Communication.cpp
 *
 *  Created on: 26 gru 2020
 *      Author: spoma
 */

#include "Communication.h"

//NETWORK
#define WIFI_SSID 							"Majkel"
#define WIFI_PSWD 							"12345678"
#define LOCAL_PORT							6000

// UDP
#define DELAY_BETWEEN_UDP_STANDARD			10		// delay between UDP send [s]
#define DELAY_BETWEEN_UDP_DIAGNOSE			30		// delay between UDP send [s]

//Fuctions
bool WiFi_conectionCheck();
void UDPsendStandardFrame();
DataRead UDPread();
void UDPsendDiagnoseFrame();
DataRead getDataRead();
void resetNewData();

//Standard variable
WiFiUDP Udp;								// WiFi variable // @suppress("Abstract class cannot be instantiated")
IPAddress broadcastIP;
bool flagConnectionWasIntrrupt = true;
DataRead dataRead;
DataWritte dataWritte;
boolean forceUDPStandardFrame = false;
int eepromSize = 0;

EEPROMClass eeprom;


//millis
unsigned long udpStandardsendMillis = 0;
unsigned long udpDiagnosesendMillis = 0;

Communication::Communication(boolean start) {
}

Communication::~Communication() {
}

void Communication::run() {
	if (!dataRead.newData)
		dataRead = UDPread();
	UDPsendStandardFrame();
	UDPsendDiagnoseFrame();
}

void Communication::WiFi_init() {
    WiFi.begin(WIFI_SSID, WIFI_PSWD);
    Serial.print("WIFI status = ");
    Serial.println(WiFi.getMode());
}

bool WiFi_conectionCheck() {
	Diagnose diagnose = getDiagnose();
	if (!WiFi.isConnected()) {
		WiFi.reconnect();
		if (diagnose.wifiConnected) {
			diagnose.wifiConnectionInterrupt++;
			diagnose.wifiConnected = false;
		}
		flagConnectionWasIntrrupt = true;
		Serial.println("Zerwane polaczenie z siecia WiFi. Ponowne laczenie");
		setDiagnose(diagnose);
		return false;
	}
	else {
		if (flagConnectionWasIntrrupt) {
			diagnose.wifiConnected = true;
		    Udp.begin(LOCAL_PORT);
		    Serial.print("Local port: ");
		    Serial.println(LOCAL_PORT);
		    // broadcast IP
		    IPAddress localIP = WiFi.localIP();
		    diagnose.ip[0] = localIP[0];
		    diagnose.ip[1] = localIP[1];
		    diagnose.ip[2] = localIP[2];
		    diagnose.ip[3] = localIP[3];

		    broadcastIP = localIP;
		    broadcastIP[3] = 255;

		    Serial.print("Local IP: ");
		    Serial.println(localIP);
		    Serial.println();
		    Serial.print("Broadcast IP: ");
		    Serial.println(broadcastIP);
		    Serial.println();

			flagConnectionWasIntrrupt = false;
			Serial.print("Polaczono z siecia ");
			Serial.println(WIFI_SSID);
		}
		setDiagnose(diagnose);
		return true;
	}
}

void forceStandardUDP() {
	forceUDPStandardFrame = true;
}

void UDPsendStandardFrame() {
	// store data write to UDP
	// return if last telegram were send under delay ago
	if ((sleep(&udpStandardsendMillis, DELAY_BETWEEN_UDP_STANDARD))
			&& (!forceUDPStandardFrame)) return;
	if (dataWritte.length == 0) return;
	if (!WiFi_conectionCheck()) return;
	forceUDPStandardFrame = false;

	Udp.beginPacket(broadcastIP, LOCAL_PORT);
	Udp.write(getModuleType());
	Udp.write(getModuleNo());
	Udp.write(dataWritte.frameNo);
	for (int i=0; i<(dataWritte.length); i++)
		Udp.write(dataWritte.data[i]);
	Udp.endPacket();

	dataWritte.length = 0;
}

DataRead UDPread() {
	DataRead dataRead;
	dataRead.length = Udp.parsePacket();

	if (dataRead.length != 0) {
		Udp.read(dataRead.data,dataRead.length);
		int deviceType = dataRead.data[0];
		int deviceNo = dataRead.data[1];
		int frameNo = dataRead.data[2];

		//Module GLOWNY
		//DateTime synchro
		if ((deviceType == 1) && (deviceNo == 0) && (frameNo == 0))
				dateTimeSet(dataRead.data[3], dataRead.data[4], dataRead.data[5], dataRead.data[6], dataRead.data[7], dataRead.data[8], dataRead.data[9]);
		else {
			dataRead.newData = true;
			dataRead.deviceType=deviceType;
			dataRead.deviceNo=deviceNo;
			dataRead.frameNo=frameNo;
			for (int i=3; i<dataRead.length; i++)
				dataRead.data[i-3] = dataRead.data[i];
			dataRead.length = dataRead.length - 3;
		}
	}
	return dataRead;
}

void UDPsendDiagnoseFrame() {
	if (sleep(&udpDiagnosesendMillis, DELAY_BETWEEN_UDP_DIAGNOSE)) return;
	if (!WiFi_conectionCheck()) return;
	byte dataWritte[7];

	// First three bytes are reserved for device recognized purposes.
	dataWritte[0] = getModuleType();
	dataWritte[1] = getModuleNo();
	dataWritte[2] = 200;						// Frame 200
	dataWritte[3] = getDiagnose().ip[0];
	dataWritte[4] = getDiagnose().ip[1];
	dataWritte[5] = getDiagnose().ip[2];
	dataWritte[6] = getDiagnose().ip[3];

	//Send data packet
	Udp.beginPacket(broadcastIP, LOCAL_PORT);
	for (int i=0; i<7; i++) {
		Udp.write(dataWritte[i]);
	}
	Udp.endPacket();

}

DataRead getDataRead() {
	return dataRead;
}

void resetNewData() {
	dataRead.newData = false;
}

void setUDPdata(int frameNo, byte *data, int length) {
	for (int i=0; i<length; i++)
		dataWritte.data[i] = data[i];
	dataWritte.frameNo = frameNo;
	dataWritte.length = length;
}

void EEpromScan(byte EEpromData[], int size) {
	eepromSize = size;
	eeprom.begin(eepromSize);						// EEPROM begin
	delay(100);
	Serial.print("Getting data from EEprom");
	for (int i=0; i<eepromSize; i++) {
		byte data = eeprom.read(i);
		EEpromData[i] = data;
		Serial.printf("\nByte[%i]=%i",i,data);
	}
}

void EEpromWrite(int pos, int value) {
	eeprom.begin(eepromSize);						// EEPROM begin
	eeprom.write(pos,value);
	eeprom.commit();
}


