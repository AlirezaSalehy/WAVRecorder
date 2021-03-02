#ifndef WAV_GENERATOR_LIBRARY
#define WAV_GENERATOR_LIBRARY

#include <SPI.h>
#include <SD.h>

#define CHUNK_SIZE 512
#define NUM_CHUNK	96
#define BUFFDER_LEN NUM_CHUNK * CHUNK_SIZE

#if defined(ESP8266) || defined(ESP32)
	#define File File
#endif


#ifdef ARDUINO_SAM_DUE 
	#define File SDLib::File
#endif


struct audio_t {
	File* dataFile;
	
	uint32_t samplingRate;
	uint16_t sampleLength;
		
	// This should be at most 2, which for stereo audio
	uint16_t numberOfChannels;
};


class WAVGenerator {

	private:
		uint8_t buffer[BUFFDER_LEN];// = {0};
		
		uint8_t toMarkFill[NUM_CHUNK];// = {0};
		uint8_t isFilled[NUM_CHUNK];// = {0};
		uint8_t* chunk = buffer;
		uint16_t index = 0;
		
		volatile uint8_t stopped = 0;
		
		uint8_t toWriteChunk = 0;
		uint32_t sampleChunkSize = 0;
		
		audio_t audio;
		HardwareSerial* serial = 0;
		File* dataFile = 0;
		
		void putInArray(uint8_t* dest, uint32_t src, uint8_t len);
		void flush();
		
		void writeHeader();
		void appenedFileSize();
		void closeFile();
		
	public:
		WAVGenerator(HardwareSerial* serial);
		WAVGenerator(audio_t audio, HardwareSerial* serial);
		void setAudio(audio_t audio);
		
		void appendBuffer(uint8_t* data, uint16_t len, uint8_t isMarked);
		void writeChunks();
		bool isBufferFull();
		void create();
		
		void markCurrentChunk();
		void markIgnoredChunks(uint16_t numSamples);
		void markAllChunks();
		
		//~WAVGenerator();
		//void appendBufferOverwrite(uint8_t* data, uint16_t len, uint8_t stat);
		
};

#endif