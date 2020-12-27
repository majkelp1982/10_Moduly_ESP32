#include "Arduino.h"
#include "Basic.h"
#include "Status.h"

String main1;
String dt ;
String statusWIFI;
String logs[30] = "0";

unsigned long lastMillis = 0;

//TMP
void setStatus(Device device, DateTime dateTime) {
	main1 = "MAIN\n";
	main1 +="Czerpnia:\t T="; main1 +=device.sensorsBME280[0].temperature; main1 +="[stC]\tH="; main1 +=(int)device.sensorsBME280[0].humidity;
	main1 +="[%]\tP="; main1 +=(int)device.sensorsBME280[0].pressure;main1 +="[hPa] Faulty="; main1 +=(int)device.sensorsBME280[0].faultyReadings ;main1 +="\n";
	main1 +="Wyrzutnia:\t T="; main1 +=device.sensorsBME280[1].temperature; main1 +="[stC]\tH="; main1 +=(int)device.sensorsBME280[1].humidity;
	main1 +="[%]\tP="; main1 +=(int)device.sensorsBME280[1].pressure;main1 +="[hPa] Faulty="; main1 +=(int)device.sensorsBME280[1].faultyReadings ;main1 +="\n";
	main1 +="Nawiew:\t\t T="; main1 +=device.sensorsBME280[2].temperature; main1 +="[stC]\tH="; main1 +=(int)device.sensorsBME280[2].humidity;
	main1 +="[%]\tP="; main1 +=(int)device.sensorsBME280[2].pressure;main1 +="[hPa] Faulty="; main1 +=(int)device.sensorsBME280[2].faultyReadings ;main1 +="\n";
	main1 +="Wywiew:\t\t T="; main1 +=device.sensorsBME280[3].temperature; main1 +="[stC]\tH="; main1 +=(int)device.sensorsBME280[3].humidity;
	main1 +="[%]\tP="; main1 +=(int)device.sensorsBME280[3].pressure;main1 +="[hPa] Faulty="; main1 +=(int)device.sensorsBME280[3].faultyReadings ;main1 +="\n";

	dt = "Data[";
	dt +=dateTime.day; dt +="."; dt +=dateTime.month; dt +="."; dt +=dateTime.year; dt +=" dT("; dt +=dateTime.weekDay; dt +=")] Czas[";
	dt +=dateTime.hour;dt +=":"; dt +=dateTime.minute; dt +=":"; dt +=dateTime.second; dt +="]";

	statusWIFI = "IP ["; statusWIFI +=device.diagnose.ip[0]; statusWIFI +=".";
	statusWIFI +=device.diagnose.ip[1]; statusWIFI +="."; statusWIFI +=device.diagnose.ip[2];
	statusWIFI +="."; statusWIFI +=device.diagnose.ip[3]; statusWIFI +="]";
	if (device.diagnose.wifiConnected) statusWIFI +=" Connected[true] Connect_interrupted["; else statusWIFI +=" Connected[false] Connect_interrupted[";
	statusWIFI +=device.diagnose.wifiConnectionInterrupt; statusWIFI +="]";

}

void addLog(String log, DateTime dateTime) {
	for (int i=28; i>=0; i--)
		logs[i+1] = logs[i];

	logs[0] = dateTime.year;
	logs[0] += ".";
	logs[0] += dateTime.month;
	logs[0] += ".";
	logs[0] += dateTime.day;
	logs[0] += " ";
	logs[0] += dateTime.hour;
	logs[0] += ":";
	logs[0] += dateTime.minute;
	logs[0] += ":";
	logs[0] += dateTime.second;
	logs[0] += " ";
	logs[0] += log;

	Serial.println(logs[0]);
}

void printStatus(Device device, DateTime dateTime) {
	if (Sleep(&lastMillis, 10)) return;
	setStatus(device, dateTime);

	Serial.print("\n\n");
	Serial.print(main1+"\n");
	Serial.print(dt+"\n");
	Serial.print(statusWIFI+"\n");
	for (int i=0; i<20; i++)
		Serial.print(logs[i]+"\n");
}

String getHTMLStatus() {
	String status;
	status = "<h1>ESP32 Diagnose</h1>";
	status += "<h2> Type ";
	status += DEVICE_TYP;
	status += "</h2>\n";
	status += "<p>";
	for (int i=0; i<main1.length(); i++)
		if (main1[i]=='\n')
				status +="</p><p>";
		else
			status += main1[i];
	status += "</p>";
	status += "<p>";
	status += dt;
	status += "</p>";
	status += "<p>";
	status += statusWIFI;
	status += "</p>";
	for (int i=0; i<30; i++) {
		status += "<p>";
		status += logs[i];
		status += "\n";
		status += "</p>";
	}
	return status;
}

