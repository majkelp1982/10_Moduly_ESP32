#include "WebService.h"
#include "Module.h"
#include <ArduinoJson.h>

WebServer server2(9090);
StaticJsonDocument<2500> doc;
String jsonDeserialization(String json);
void webServerStart();
void webServerSetup();

void webService_setup() {
    webServerStart();
    webServerSetup();
}

void webServerStart() {
  	  /*use mdns for host name resolution*/
	  if (!MDNS.begin("esp32")) { //http://esp32.local
	    Serial.println("Error setting up MDNS responder!");
	    while (1) {
	      delay(1000);
	    }
	  }
	  Serial.println("mDNS responder started");
}

//Endpoints
// /   					GET		- login page
// /serverIndex 		GET		- upload page
// /update				GET		- handlin firmware OTA file
// /configuration		POST	- save configuration
// /action				POST	- handling pins/sensors action
// /params				GET		- retvieve macAddress, connection interruptions, etc.
// /unlockType			POST	- To change modul type, type need to be unlocked

void webServerSetup() {
	  //Service
	   server2.on("/action", HTTP_POST, []() {
		if (server2.hasArg("plain") == false) {
			server2.send(404, "text/plain", "JSON NOT FOUND");
			return;
		}
		jsonDeserialization(server2.arg("plain"));
	   });
	  server2.begin();
}

void webService_run() {
    server2.handleClient();
}

String jsonDeserialization(String json) {
	  doc.clear();
	  doc.garbageCollect();
	  DeserializationError error = deserializeJson(doc, json);
	  if (error) {
	    Serial.print(F("deserializeJson() failed: "));
	    Serial.println(error.f_str());
	    return "Deserialization failed";
	  }
	  int zoneNumber = doc["zoneNumber"];
	  float temperature = doc["temperature"];
	  int humidity = doc["humidity"];
	  Serial.print("\nGet zone ");
	  Serial.print(zoneNumber);
	  Serial.print(" temp:");
	  Serial.print(temperature);
	  Serial.print(" humidity:");
	  Serial.println(humidity);
	  getDevice().zone[zoneNumber].isTempFromService = temperature;
	  getDevice().zone[zoneNumber].humidityFromService = humidity;
	  return "OK";
}
