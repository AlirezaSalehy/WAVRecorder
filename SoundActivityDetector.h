#ifndef SOUND_ACTIVITY_DETECTION_LIBRARY
#define SOUND_ACTIVITY_DETECTION_LIBRARY

#include <SD.h>

#ifdef ESP8266
	// Here we are to use timer 1
    extern "C" {
        #include "user_interface.h"
    }

#elif ESP32
	#include <HardwareSerial.h>
#endif

#include "AudioTimer.h"
//#include "WAVRecorder.h"


// Note that SoundActivityDetection uses timerX
class SoundActivityDetector {

	private:
		uint8_t upper_threshold = 0;
		uint8_t lower_threshold = 0;

		uint16_t minimumLengthNS = 0;
		uint16_t maximumRewardNS = 0;

	public:
		SoundActivityDetector(uint8_t adcPin, uint16_t num_samples, uint16_t minimumLengthNS, uint16_t maximumRewardNS, HardwareSerial* serial);

		// This should be called at first, for example when constructor is called
		void calibrate(uint16_t num_samples);
		void calibrate(uint16_t averageVector, uint16_t averageValue);

		void setThresholds(uint8_t lower, uint8_t upper);

		// This should be called in the ISR, it should be implemented in way that has some memory
		// This could be called when buffer is going to be writen, too.
		bool detect(uint16_t dat);
		bool detect(uint16_t* buffer, uint16_t len);

		uint16_t getMinimumLengthNS();
};

#endif