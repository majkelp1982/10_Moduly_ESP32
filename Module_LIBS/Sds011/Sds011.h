// SDS011 dust sensor PM2.5 and PM10
// ---------------------------------
//
// By R. Zschiegner (rz@madavi.de)
// April 2016
//
// Documentation:
//		- The iNovaFitness SDS011 datasheet
//

#include "Arduino.h"

// SDS011 MODULE STATE
#define MODE_WAKEUP							1
#define MODE_SLEEP							2

class SDS011 {
	public:
		SDS011();
		SDS011(uint8_t pin_rx, uint8_t pin_tx);
		void begin(void);
		int read(float *p25, float *p10);
		void sleep();
		void wakeup();
		unsigned long modeTime=0;
		int mode;
	private:
		uint8_t _pin_rx, _pin_tx;

};

//Added by Sloeber 
#pragma once
