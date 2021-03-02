#ifndef LUT_CALIBRATED_ADC_LIBRARY
#define LUT_CALIBRATED_ADC_LIBRARY

#include <SD.h>

#if defined(ESP8266) || defined(ESP32)
	#define File File
#endif


#ifdef ARDUINO_SAM_DUE 
	#define File SDLib::File
#endif

class LUTCalibratedADC
{
	private:
		uint16_t lookUpTable[256];
		uint8_t DACChannel;
		uint8_t ADCChannel;
		HardwareSerial* serial;
		
	public:
		LUTCalibratedADC(uint8_t DACChannel, uint8_t ADCChannel, HardwareSerial* serial);
		void createLookTable(uint8_t numSamples, uint8_t ADCResolution);
		void saveOnFileCSV(File* stream);
		void writeOnStream(Stream* stream);
		
};


#endif