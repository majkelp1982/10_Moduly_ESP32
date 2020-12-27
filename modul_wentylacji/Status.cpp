#include "Arduino.h"
#include "Basic.h"
#include "Status.h"

String main1;
String dt ;
String statusWIFI;
String logs[30] = "0";

unsigned long lastMillis = 0;

//TMP
void setStatus(Device device, Humidity humidity, Temperature temperature, DateTime dateTime) {
	main1 = "MAIN";
	main1 +="	Fan["; (device.fan) ? main1 +="true": main1 +="false"; main1 +="]";
	main1 +="	Pump["; (device.pump) ? main1 +="true": main1 +="false"; main1 +="]";
	main1 +="	H01["; main1 +=device.hour[0]; main1 +="]";
	main1 +="	H23["; main1 +=device.hour[1]; main1 +="]";
	main1 +="	H45["; main1 +=device.hour[2]; main1 +="]";
	main1 +="	H67["; main1 +=device.hour[3]; main1 +="]";
	main1 +="	H89["; main1 +=device.hour[4]; main1 +="]";
	main1 +="	H1011["; main1 +=device.hour[5]; main1 +="]";
	main1 +="	H1213["; main1 +=device.hour[6]; main1 +="]";
	main1 +="	H1415["; main1 +=device.hour[7]; main1 +="]";
	main1 +="	H1617["; main1 +=device.hour[8]; main1 +="]";
	main1 +="	H1819["; main1 +=device.hour[9]; main1 +="]";
	main1 +="	H2021["; main1 +=device.hour[10]; main1 +="]";
	main1 +="	H2223["; main1 +=device.hour[11]; main1 +="]";
	main1 +="\n";

	main1 += "EVENT";
	main1 +="	NormalOn["; main1 +=device.normalOn; main1 +="]";
	main1 +="	humAlert["; main1 +=device.humidityAlert; main1 +="]";
	main1 +="\n";

	main1 += "HUMIDITY";
	main1 +="	Hsalon["; main1 +=humidity.salon.is; main1 +="]";
	main1 +="	Hlazdol["; main1 +=humidity.lazDol.is; main1 +="]";
	main1 +="	Hpralnia["; main1 +=humidity.pralnia.is; main1 +="]";
	main1 +="	Hrodzice["; main1 +=humidity.rodzice.is; main1 +="]";
	main1 +="	HNatalia["; main1 +=humidity.natalia.is; main1 +="]";
	main1 +="	HKarolina["; main1 +=humidity.karolina.is; main1 +="]";
	main1 +="	HlazGora["; main1 +=humidity.lazGora.is; main1 +="]";
	main1 +="\n";

	main1 += "TEMPERATURE";
	main1 +="	Tsalon["; 	main1 +=temperature.salon.is; main1 +="]["; main1 += temperature.salon.req; main1 +="]";
	main1 +="	TlazDol["; 	main1 +=temperature.lazDol.is; main1 +="]["; main1 += temperature.lazDol.req; main1 +="]";
	main1 +="	Tpralnia["; 	main1 +=temperature.pralnia.is; main1 +="]["; main1 += temperature.pralnia.req; main1 +="]";
	main1 +="	Trodzice["; 	main1 +=temperature.rodzice.is; main1 +="]["; main1 += temperature.rodzice.req; main1 +="]";
	main1 +="	Tnatalia["; 	main1 +=temperature.natalia.is; main1 +="]["; main1 += temperature.natalia.req; main1 +="]";
	main1 +="	Tkarolina["; 	main1 +=temperature.karolina.is; main1 +="]["; main1 += temperature.karolina.req; main1 +="]";
	main1 +="	TlazGora["; 	main1 +=temperature.lazGora.is; main1 +="]["; main1 += temperature.lazGora.req; main1 +="]";



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

void printStatus(Device device, Humidity humidity, Temperature temperature, DateTime dateTime) {
	if (Sleep(&lastMillis, 10)) return;
	setStatus(device, humidity, temperature, dateTime);

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
	status += main1;
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

