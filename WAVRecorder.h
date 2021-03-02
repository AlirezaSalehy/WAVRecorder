#ifndef WAV_RECORDER_LIBRARY
#define WAV_RECORDER_LIBRARY

#include <SPI.h>
#include <SD.h>


#include "SoundActivityDetector.h"
#include "WAVGenerator.h"
#include "AudioTimer.h"

struct channel_t {
	// The ADC pin of the microphone
	uint8_t ADCPin;	
};

// Note that WAVRecorder uses timerX
class WAVRecorder {

	private:
		//uint32_t samplingRate = 0;
		//HardwareSerial* serial = 0;
		//WAVGenerator* wg = 0;
		
		//volatile uint32_t counter = 0;
		
		AudioTimer* timer = 0;
		
	public:
		WAVRecorder(uint8_t adcDatLen, channel_t* channels, uint16_t numChannels, uint32_t samplingRate, uint16_t sampleLength, HardwareSerial* serial);
		WAVRecorder(uint8_t adcDatLen, channel_t* channels, uint16_t numChannels, File* dataFile, uint32_t samplingRate, uint16_t sampleLength, HardwareSerial* serial);
		void start(uint32_t time);
		void start();
		void start(SoundActivityDetector* sad_arg);
		void startBlocking(uint32_t time);
		void startBlocking(SoundActivityDetector* sad_arg);
		void stop();
		
		void setFile(File* dataFile);
		//~WAVRecorder();
};

#endif