#include "Basic.h"

void WiFi_init();
void UDPsendStandardFrame(Device *device);
void UDPread(Device *device, DateTime *dateTime);
void UDPsendDiagnoseFrame(Device *device);
