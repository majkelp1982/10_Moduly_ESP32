#include "Arduino.h"
#include "Communication.h"
#include "Basic.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "EepromEvent.h"

WiFiUDP Udp;								// WiFi variable @suppress("Abstract class cannot be instantiated")
IPAddress broadcastIP;
bool flagConnectionWasIntrrupt = true;

byte dataRead[128];							// store data read from UDP
byte dataWrite[128];						// store data write to UDP

//millis
unsigned long udpStandardsendMillis = 0;
unsigned long udpDiagnosesendMillis = 0;

void WiFi_init() {
    //DO NOT TOUCH
    //  This is here to force the ESP32 to reset the WiFi and initialize correctly.
    Serial.print("WIFI status = ");
    Serial.println(WiFi.getMode());
    WiFi.disconnect(true);
    delay(1000);
    WiFi.mode(WIFI_STA);
    delay(1000);
    Serial.print("WIFI status = ");
    Serial.println(WiFi.getMode());
    // End silly stuff !!!	// Begin WiFi connection
	WiFi.begin(WIFI_SSID, WIFI_PSWD);
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
void UDPSetHumitidty(Humidity *humidity) {
	unsigned long currentMillis = millis();
	if (currentMillis-humidity->salon.lastUpdate > DELAY_UDP_SAVE) {
		humidity->salon.is = dataRead[6];
		humidity->salon.lastUpdate= currentMillis;
	}
	if (currentMillis-humidity->lazDol.lastUpdate > DELAY_UDP_SAVE) {
		humidity->lazDol.is = dataRead[10];
		humidity->lazDol.lastUpdate= currentMillis;
	}
	if (currentMillis-humidity->pralnia.lastUpdate > DELAY_UDP_SAVE) {
		humidity->pralnia.is = dataRead[14];
		humidity->pralnia.lastUpdate= currentMillis;
	}
	if (currentMillis-humidity->rodzice.lastUpdate > DELAY_UDP_SAVE) {
		humidity->rodzice.is = dataRead[18];
		humidity->rodzice.lastUpdate= currentMillis;
	}
	if (currentMillis-humidity->natalia.lastUpdate > DELAY_UDP_SAVE) {
		humidity->natalia.is = dataRead[22];
		humidity->natalia.lastUpdate= currentMillis;
	}
	if (currentMillis-humidity->karolina.lastUpdate > DELAY_UDP_SAVE) {
		humidity->karolina.is = dataRead[26];
		humidity->karolina.lastUpdate= currentMillis;
	}
	if (currentMillis-humidity->lazGora.lastUpdate > DELAY_UDP_SAVE) {
		humidity->lazGora.is = dataRead[30];
		humidity->lazGora.lastUpdate= currentMillis;
	}
}

void UDPSetTemperature(Temperature *temperature) {
	unsigned long currentMillis = millis();
	if (currentMillis-temperature->salon.lastUpdate > DELAY_UDP_SAVE) {
		temperature->salon.is = dataRead[3]+(dataRead[4]/10);
		temperature->salon.req= dataRead[5]/2;
		temperature->salon.lastUpdate = currentMillis;
	}
	if (currentMillis-temperature->lazDol.lastUpdate > DELAY_UDP_SAVE) {
		temperature->lazDol.is = dataRead[7]+(dataRead[8]/10);
		temperature->lazDol.req= dataRead[9]/2;
		temperature->lazDol.lastUpdate = currentMillis;
	}
	if (currentMillis-temperature->pralnia.lastUpdate > DELAY_UDP_SAVE) {
		temperature->pralnia.is = dataRead[11]+(dataRead[12]/10);
		temperature->pralnia.req= dataRead[13]/2;
		temperature->pralnia.lastUpdate = currentMillis;
	}
	if (currentMillis-temperature->rodzice.lastUpdate > DELAY_UDP_SAVE) {
		temperature->rodzice.is = dataRead[15]+(dataRead[16]/10);
		temperature->rodzice.req= dataRead[17]/2;
		temperature->rodzice.lastUpdate = currentMillis;
	}
	if (currentMillis-temperature->natalia.lastUpdate > DELAY_UDP_SAVE) {
		temperature->natalia.is = dataRead[19]+(dataRead[20]/10);
		temperature->natalia.req= dataRead[21]/2;
		temperature->natalia.lastUpdate = currentMillis;
	}
	if (currentMillis-temperature->karolina.lastUpdate > DELAY_UDP_SAVE) {
		temperature->karolina.is = dataRead[23]+(dataRead[24]/10);
		temperature->karolina.req= dataRead[25]/2;
		temperature->karolina.lastUpdate = currentMillis;
	}
	if (currentMillis-temperature->lazGora.lastUpdate > DELAY_UDP_SAVE) {
		temperature->lazGora.is = dataRead[27]+(dataRead[28]/10);
		temperature->lazGora.req= dataRead[29]/2;
		temperature->lazGora.lastUpdate = currentMillis;
	}
}


void UDPsendStandardFrame(Device *device) {
	// return if last telegram were send under delay ago
	if (Sleep(&udpStandardsendMillis, DELAY_BETWEEN_UDP_STANDARD)) return;
	if (!WiFi_conectionCheck(device)) return;

	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = DEVICE_TYP;
	dataWrite[1] = DEVICE_NO;
	dataWrite[2] = 0;						// Frame 0
	dataWrite[3] = (device->fan << 7);
	dataWrite[4] = device->hour[0];
	dataWrite[5] = device->hour[1];
	dataWrite[6] = device->hour[2];
	dataWrite[7] = device->hour[3];
	dataWrite[8] = device->hour[4];
	dataWrite[9] = device->hour[5];
	dataWrite[10] = device->hour[6];
	dataWrite[11] = device->hour[7];
	dataWrite[12] = device->hour[8];
	dataWrite[13] = device->hour[9];
	dataWrite[14] = device->hour[10];
	dataWrite[15] = device->hour[11];

	//Send data packet
	Udp.beginPacket(broadcastIP, LOCAL_PORT);
	for (int i=0; i<=FRAME_SIZE; i++) {
		Udp.write(dataWrite[i]);
	}
	Udp.endPacket();
}

void UDPread(Device *device, Humidity *humidity, Temperature *temperature, DateTime *dateTime) {
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
			if (dataRead[3] == 4) {
				device->hour[0] = dataRead[4];
				EEpromWrite(0, dataRead[4]);
			}
			if (dataRead[3] == 5) {
				device->hour[1] = dataRead[4];
				EEpromWrite(1, dataRead[4]);
			}
			if (dataRead[3] == 6) {
				device->hour[2] = dataRead[4];
				EEpromWrite(2, dataRead[4]);
			}
			if (dataRead[3] == 7) {
				device->hour[3] = dataRead[4];
				EEpromWrite(3, dataRead[4]);
			}
			if (dataRead[3] == 8) {
				device->hour[4] = dataRead[4];
				EEpromWrite(4, dataRead[4]);
			}
			if (dataRead[3] == 9) {
				device->hour[5] = dataRead[4];
				EEpromWrite(5, dataRead[4]);
			}
			if (dataRead[3] == 10) {
				device->hour[6] = dataRead[4];
				EEpromWrite(6, dataRead[4]);
			}
			if (dataRead[3] == 11) {
				device->hour[7] = dataRead[4];
				EEpromWrite(7, dataRead[4]);
			}
			if (dataRead[3] == 12) {
				device->hour[8] = dataRead[4];
				EEpromWrite(8, dataRead[4]);
			}
			if (dataRead[3] == 13) {
				device->hour[9] = dataRead[4];
				EEpromWrite(9, dataRead[4]);
			}
			if (dataRead[3] == 14) {
				device->hour[10] = dataRead[4];
				EEpromWrite(10, dataRead[4]);
			}
			if (dataRead[3] == 15) {
				device->hour[11] = dataRead[4];
				EEpromWrite(11, dataRead[4]);
			}

			udpStandardsendMillis = 0;
		}
	}

	if (deviceTyp == 2) {
		//standard frame
		if (frameNo == 0) {
			UDPSetHumitidty(humidity);
			UDPSetTemperature(temperature);
		}
	}
}

void UDPsendDiagnoseFrame(Device *device) {
	if (Sleep(&udpDiagnosesendMillis, DELAY_BETWEEN_UDP_DIAGNOSE)) return;
	if (!WiFi_conectionCheck(device)) return;

	//TODO
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


