#ifndef WALLE_TIMER_LIBRARY
#define WALLE_TIMER_LIBRARY

#ifdef ESP32
	#include "esp32-hal.h"
#endif

#ifdef ESP8266
	// Here we are to use timer 1
    extern "C" {
        #include "user_interface.h"
    }
	
	#include <ESP8266WiFi.h>

#endif

#ifdef ARDUINO_SAM_DUE 
	// Here we are going to use timer 4
	#include <DueTimer.h>
#endif

class AudioTimer {
	private:
	
	public:
	void setup(uint32_t frequency, void (*f)());
	void start();
	void stop();
};


#endif