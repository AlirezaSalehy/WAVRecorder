#ifndef AUDIO_SYSTEM_LIBRARY
#define AUDIO_SYSTEM_LIBRARY

	#if defined(ESP32) || defined(ESP8266)
	
		#include <Arduino.h>
		#include <SPI.h>
		#include <SD.h>
		
		#include <AudioFileSourceSD.h>
		#include <AudioOutput.h>
		#include <AudioOutputI2S.h>
		#include <AudioOutputI2SNoDAC.h>
		#include <AudioGeneratorWAV.h>
		#include <AudioGeneratorMP3.h>
		#include <AudioGenerator.h>
		
		#ifdef ESP8266
			#define SPDIF_OUT_PIN 27
		#endif
		
		#ifdef ESP32
		    #include <WiFi.h>
			#include "esp32-hal.h"
		#endif
		
		
		#define EXTERNAL_DAC 0
		#define INTERNAL_DAC 1
		#define INTERNAL_PDM 2
		
		// A wrapper class for ESP8266Audio library
		class AudioSystem {
		  
		  private:
		    AudioFileSourceSD *source = NULL;
			AudioOutput *output = NULL;
			AudioGenerator *decoder = NULL;
			
			AudioSystem();
			
			//void ICACHE_RAM_ATTR onTimerISR();
			 
		  public:
		  	// Chip Select pin == csPin
		  	AudioSystem(uint8_t csPin);
			
		    // API for playing audio in blocking manner
			void playAudioBlocking(char* fileName);
			
			// API for recording voice in blocking manner
			File recordAudioBlocking(uint32_t milisec, uint16_t frquency);
			
			void playAudioLoop();
		};
	
	#endif

#endif