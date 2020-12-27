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
void UDPsendStandardFrame(byte data[]);
DataRead UDPread();
void UDPsendDiagnoseFrame();
DataRead getDataRead();
void resetNewData();

//Standard variable
WiFiUDP Udp;								// WiFi variable // @suppress("Abstract class cannot be instantiated")
IPAddress broadcastIP;
bool flagConnectionWasIntrrupt = true;
DataRead dataRead;

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

void UDPsendStandardFrame(byte data[]) {
	int arraySize = sizeof(data)/sizeof(data[0]);
	byte dataWrite[arraySize+3];
	// store data write to UDP
	// return if last telegram were send under delay ago
	if (sleep(&udpStandardsendMillis, DELAY_BETWEEN_UDP_STANDARD)) return;
	if (!WiFi_conectionCheck()) return;

	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = getModuleType();
	dataWrite[1] = getModuleNo();
	dataWrite[2] = 0;						// Frame 0

	for (int i=0; i<arraySize; i++) {
		dataWrite[i+3] = data[i];
	}

	//Send data packet
	Udp.beginPacket(broadcastIP, LOCAL_PORT);
	for (int i=0; i<=(arraySize+3); i++) {
		Udp.write(dataWrite[i]);
	}
	Udp.endPacket();
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
	byte dataWrite[7];

	//TODO
	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = getModuleType();
	dataWrite[1] = getModuleNo();
	dataWrite[2] = 200;						// Frame 200
	dataWrite[3] = getDiagnose().ip[0];
	dataWrite[4] = getDiagnose().ip[1];
	dataWrite[5] = getDiagnose().ip[2];
	dataWrite[6] = getDiagnose().ip[3];

	//Send data packet
	Udp.beginPacket(broadcastIP, LOCAL_PORT);
	for (int i=0; i<=7; i++) {
		Udp.write(dataWrite[i]);
	}
	Udp.endPacket();

}

DataRead getDataRead() {
	return dataRead;
}

void resetNewData() {
	dataRead.newData = false;
}



