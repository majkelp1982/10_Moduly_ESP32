#include "Arduino.h"
#include "Communication.h"
#include "Basic.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "EepromEvent.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//Help functions
byte UDPreturnTemperatureTosend(byte lastByteSend, float temperature);

//Standard variable
WiFiUDP Udp;								// WiFi variable // @suppress("Abstract class cannot be instantiated")
IPAddress broadcastIP;
bool flagConnectionWasIntrrupt = true;

byte dataRead[128];							// store data read from UDP
byte dataWrite[128];						// store data write to UDP

//millis
unsigned long udpStandardsendMillis = 0;
unsigned long udpDiagnosesendMillis = 0;

void WiFi_init() {
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
	    device->diagnose.ip[0] = 0;
	    device->diagnose.ip[1] = 0;
	    device->diagnose.ip[2] = 0;
	    device->diagnose.ip[3] = 0;

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

	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = DEVICE_TYP;
	dataWrite[1] = DEVICE_NO;
	dataWrite[2] = 0;						// Frame 0
	dataWrite[3] = (device->heatSourceActive << 6) | (device->pump_InHouse << 5) | (device->pump_UnderGround << 4) | (device->reqHeatPumpOn << 3) |
			(device->cheapTariffOnly << 2) | (device->heatingActivated << 1) | (device->antyLegionellia) << 0;

	dataWrite[4] = (device->valve_3way << 6) | (device->valve_bypass << 0);
	dataWrite[5] = (device->zone[0].circuit << 7) | (device->zone[1].circuit << 6) | (device->zone[2].circuit << 5) | (device->zone[3].circuit << 4)
					| (device->zone[4].circuit << 3) | (device->zone[5].circuit << 2) | (device->zone[6].circuit << 1);
	dataWrite[6] = UDPreturnTemperatureTosend(dataWrite[7], device->reqTempBuforCO);
	dataWrite[7] = UDPreturnTemperatureTosend(dataWrite[7], device->reqTempBuforCWU);

	dataWrite[8] = UDPreturnTemperatureTosend(dataWrite[7], device->tBuffCOdol.isTemp);
	dataWrite[9] = UDPreturnTemperatureTosend(dataWrite[7], device->tBuffCOsrodek.isTemp);
	dataWrite[10] = UDPreturnTemperatureTosend(dataWrite[7], device->tBuffCOgora.isTemp);

	dataWrite[11] = UDPreturnTemperatureTosend(dataWrite[7], device->tBuffCWUdol.isTemp);
	dataWrite[12] = UDPreturnTemperatureTosend(dataWrite[7], device->tBuffCWUsrodek.isTemp);
	dataWrite[13] = UDPreturnTemperatureTosend(dataWrite[7], device->tBuffCWUgora.isTemp);

	dataWrite[14] = UDPreturnTemperatureTosend(dataWrite[7], device->tZasilanie.isTemp);
	dataWrite[15] = UDPreturnTemperatureTosend(dataWrite[7], device->tPowrot.isTemp);
	dataWrite[16] = UDPreturnTemperatureTosend(dataWrite[7], device->tDolneZrodlo.isTemp);

	dataWrite[17] = UDPreturnTemperatureTosend(dataWrite[7], device->tKominek.isTemp);
	dataWrite[18] = UDPreturnTemperatureTosend(dataWrite[7], device->tRozdzielacze.isTemp);
	dataWrite[19] = UDPreturnTemperatureTosend(dataWrite[7], device->tPowrotParter.isTemp);
	dataWrite[20] = UDPreturnTemperatureTosend(dataWrite[7], device->tPowrotPietro.isTemp);
	dataWrite[21] = device->heatPumpAlarmTemperature;
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

		//dataRead[3] numer bajtu do aktualizacji
		if ((moduleTyp == DEVICE_TYP) && (modueNo == DEVICE_NO)) {

			//Send frame immediately after data received
			udpStandardsendMillis = 0;

			if (dataRead[3] == 3) {
				device->cheapTariffOnly = UDPbitStatus(dataRead[4],2);
				device->heatingActivated = UDPbitStatus(dataRead[4],1);
				device->antyLegionellia = UDPbitStatus(dataRead[4],0);
				//save value to eeprom
				EEpromWrite(1, device->cheapTariffOnly);
				EEpromWrite(2, device->heatingActivated);
			}
			if (dataRead[3] == 6) {
				device->reqTempBuforCO = (float)(dataRead[4])/2;
				//save value to eeprom
				EEpromWrite(3, dataRead[4]);
			}
			if (dataRead[3] == 7) {
				device->reqTempBuforCWU = (float)dataRead[4]/2;
				//save value to eeprom
				EEpromWrite(4, dataRead[4]);
			}
			if (dataRead[3] == 21) {
				device->heatPumpAlarmTemperature = dataRead[4];
				//save value to eeprom
				EEpromWrite(5, dataRead[4]);
			}
		}
	}

	// data from module Komfort
	if ((deviceTyp == 10) && (deviceNo==0) ){
		//standard frame
		if (frameNo == 0) {
			device->zone[0].isTemp = dataRead[3]+dataRead[4]/10.0;
			device->zone[0].reqTemp = dataRead[5]/2.0;
			device->zone[1].isTemp = dataRead[7]+dataRead[8]/10.0;
			device->zone[1].reqTemp = dataRead[9]/2.0;
			device->zone[2].isTemp = dataRead[11]+dataRead[12]/10.0;
			device->zone[2].reqTemp = dataRead[13]/2.0;
			device->zone[3].isTemp = dataRead[15]+dataRead[16]/10.0;
			device->zone[3].reqTemp = dataRead[17]/2.0;
			device->zone[4].isTemp = dataRead[19]+dataRead[20]/10.0;
			device->zone[4].reqTemp = dataRead[21]/2.0;
			device->zone[5].isTemp = dataRead[23]+dataRead[24]/10.0;
			device->zone[5].reqTemp = dataRead[25]/2.0;
			device->zone[6].isTemp = dataRead[27]+dataRead[28]/10.0;
			device->zone[6].reqTemp = dataRead[29]/2.0;

			if ((dateTime->hour >= 22 ) || (dateTime->hour < 6) ||
				((dateTime->hour >=13) && (dateTime->hour <15)) ||
				(dateTime->weekDay==6) || (dateTime->weekDay==0))		// cheap tariff at the weekend as well 6-Saturday 0-Sunday
			{
				device->zone[0].reqTemp += 0.5;
				device->zone[1].reqTemp += 0.5;
				device->zone[2].reqTemp += 0.5;
				device->zone[3].reqTemp += 0.5;
				device->zone[4].reqTemp += 0.5;
				device->zone[5].reqTemp += 0.5;
				device->zone[6].reqTemp += 0.5;
			}

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
	dataWrite[7] = device->diagnose.wifiConnectionInterrupt;

	//Send data packet
	Udp.beginPacket(broadcastIP, LOCAL_PORT);
	for (int i=0; i<=FRAME_DIAGNOSE_SIZE; i++) {
		Udp.write(dataWrite[i]);
	}
	Udp.endPacket();

}


// Help functions
byte UDPreturnTemperatureTosend(byte lastByteSend, float temperature) {
	float previousTemp = lastByteSend / 2.00;
	byte temp = temperature * 2;
	if (abs(previousTemp-temperature) >= 0.5) return temp;
	else return lastByteSend;
}

