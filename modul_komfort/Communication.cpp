#include "Arduino.h"
#include "Communication.h"
#include "Basic.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "EepromEvent.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Status.h"

//Standart variable

WiFiUDP Udp;								// WiFi variable // @suppress("Abstract class cannot be instantiated")
IPAddress broadcastIP;
bool flagConnectionWasIntrrupt = true;

byte dataRead[128];							// store data read from UDP
byte dataWrite[128];						// store data write to UDP

//millis
unsigned long udpStandardsendMillis = 0;
unsigned long udpDiagnosesendMillis = 0;

void WiFi_init() {
    //DO NOT TOUCH
    //  This is here to force the ESP32 to reset the WiFi and initialise correctly.
 //   Serial.print("WIFI status = ");
//    Serial.println(WiFi.getMode());
//    WiFi.disconnect(true);
//   delay(1000);
//   WiFi.mode(WIFI_STA);
//   delay(1000);
    // End silly stuff !!!	// Begin WiFi connection
	WiFi.begin(WIFI_SSID, WIFI_PSWD);
    Serial.print("WIFI status = ");
    Serial.println(WiFi.getMode());
}

bool WiFi_conectionCheck(Device *device) {
	if (!WiFi.isConnected()) {
		WiFi.reconnect();
		if (device->diagnose.wifiConnected) {
			device->diagnose.wifiConnectionInterrupt++;
			device->diagnose.wifiConnected = false;
		}
		flagConnectionWasIntrrupt = true;
		Serial.println("Zerwane polaczenie z siecia WiFi. Ponowne laczenie");
		return false;
	}
	else {
		if (flagConnectionWasIntrrupt) {
			device->diagnose.wifiConnected = true;
		    Udp.begin(LOCAL_PORT);
		    Serial.print("Local port: ");
		    Serial.println(LOCAL_PORT);
		    // broadcast IP
		    IPAddress localIP = WiFi.localIP();
		    device->diagnose.ip[0] = localIP[0];
		    device->diagnose.ip[1] = localIP[1];
		    device->diagnose.ip[2] = localIP[2];
		    device->diagnose.ip[3] = localIP[3];

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
		return true;
	}
}

void UDPsendStandardFrame(Device *device) {
	// return if last telegram were send under delay ago
	if (Sleep(&udpStandardsendMillis, DELAY_BETWEEN_UDP_STANDARD)) return;
	if (!WiFi_conectionCheck(device)) return;

	byte temperature1_0 = 0;
	byte temperature0_1 = 0;

	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = DEVICE_TYP;
	dataWrite[1] = DEVICE_NO;
	dataWrite[2] = 0;						// Frame 0

	for (int i=0; i<7; i++) {
		float temperature = device->zone[i].isTemp;

		temperature1_0 = (int)temperature;
		int temp=int(temperature*10);
		int temp1 = temperature1_0*10;
		int temp2 = temp-temp1;
		temperature0_1 = temp2;

		dataWrite[(i*4)+3] = temperature1_0;
		dataWrite[(i*4)+4] = temperature0_1;
		dataWrite[(i*4)+5] = device->zone[i].reqTemp*2;
		dataWrite[(i*4)+6] = device->zone[i].humidity;

		//datawrite[30] ostatni
	}

	//Send data packet
	Udp.beginPacket(broadcastIP, LOCAL_PORT);
	for (int i=0; i<=FRAME_STANDARD_SIZE; i++) {
		Udp.write(dataWrite[i]);
	}
	Udp.endPacket();
}

void UDPread(Device *device, DateTime *dateTime) {
	int packetLength = Udp.parsePacket();

	if (packetLength == 0) return;					// if no packet incoming return

	Udp.read(dataRead,packetLength);
	int deviceTyp = dataRead[0];
	int deviceNo = dataRead[1];
	int frameNo = dataRead[2];

	//Module GLOWNY
	if (deviceTyp == 1) {
		int moduleTyp = dataRead[1];
		int modueNo = dataRead[2];

		//DateTime synchro
	if ((moduleTyp == 0) && (modueNo == 0)) DateTimeSet(dateTime, dataRead[3], dataRead[4], dataRead[5], dataRead[6], dataRead[7], dataRead[8], dataRead[9]);
		if ((moduleTyp == DEVICE_TYP) && (modueNo == DEVICE_NO)) {
			//dataRead[3] numer bajtu do aktualizacji
			if (dataRead[3] == 5) {
				device->zone[0].reqTemp = (float)dataRead[4]/2;
				EEpromWrite(0, dataRead[4]);
				addLog("Zmiana temperatury - Salon", *dateTime);
			}
			if (dataRead[3] == 9) {
				device->zone[1].reqTemp = (float)dataRead[4]/2;
				EEpromWrite(1, dataRead[4]);
				addLog("Zmiana temperatury - Pralnia", *dateTime);
			}
			if (dataRead[3] == 13) {
				device->zone[2].reqTemp = (float)dataRead[4]/2;
				EEpromWrite(2, dataRead[4]);
				addLog("Zmiana temperatury - Laz. Dol", *dateTime);
			}
			if (dataRead[3] == 17) {
				device->zone[3].reqTemp = (float)dataRead[4]/2;
				EEpromWrite(3, dataRead[4]);
				addLog("Zmiana temperatury - Rodzice", *dateTime);
			}
			if (dataRead[3] == 21) {
				device->zone[4].reqTemp = (float)dataRead[4]/2;
				EEpromWrite(4, dataRead[4]);
				addLog("Zmiana temperatury - Natalia", *dateTime);
			}
			if (dataRead[3] == 25) {
				device->zone[5].reqTemp = (float)dataRead[4]/2;
				EEpromWrite(5, dataRead[4]);
				addLog("Zmiana temperatury - Karolina", *dateTime);
			}
			if (dataRead[3] == 29) {
				device->zone[6].reqTemp = (float)dataRead[4]/2;
				EEpromWrite(6, dataRead[4]);
				addLog("Zmiana temperatury - Laz.Gora", *dateTime);
			}
			udpStandardsendMillis = 0;
		}
	}
}

void UDPsendDiagnoseFrame(Device *device) {
	if (Sleep(&udpDiagnosesendMillis, DELAY_BETWEEN_UDP_DIAGNOSE)) return;
	if (!WiFi_conectionCheck(device)) return;

	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = DEVICE_TYP;
	dataWrite[1] = DEVICE_NO;
	dataWrite[2] = 200;						// Frame 200
	dataWrite[3] = device->diagnose.ip[0];
	dataWrite[4] = device->diagnose.ip[1];
	dataWrite[5] = device->diagnose.ip[2];
	dataWrite[6] = device->diagnose.ip[3];

	//Send data packet
	Udp.beginPacket(broadcastIP, LOCAL_PORT);
	for (int i=0; i<=FRAME_DIAGNOSE_SIZE; i++) {
		Udp.write(dataWrite[i]);
	}
	Udp.endPacket();

}

