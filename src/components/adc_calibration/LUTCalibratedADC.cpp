#include "LUTCalibratedADC.h"

// Based on original work from Helmut Weber (https://github.com/MacLeod-D/ESP32-ADC)
// that he described at https://esp32.com/viewtopic.php?f=19&t=2881&start=30#p47663
// Modified with bug-fixed by Henry Cheung
//
// Build a ESP32 ADC Lookup table to correct ESP32 ADC linearity issue
// Run this sketch to build your own LUT for each of your ESP32, copy and paste the
// generated LUT to your sketch for using it, see example sketch on how to use it

LUTCalibratedADC::LUTCalibratedADC(uint8_t DACChannel, uint8_t ADCChannel, HardwareSerial* serial)
{
	this->DACChannel = DACChannel;
	this->ADCChannel = ADCChannel;
	this->serial = serial;
	
	pinMode(ADCChannel, INPUT);
}

void LUTCalibratedADC::createLookTable(uint8_t numSamples, uint8_t ADCResolution)
{
    analogReadResolution(ADCResolution);

   	for (uint16_t i = 0; i < 255; i++)
	{
		dacWrite(DACChannel, 0xff & i);
		uint32_t sampleAverage = 0;
		
		for (uint16_t j = 0; j < numSamples; j++)
		{
			sampleAverage += analogRead(ADCChannel);
		} 
		sampleAverage = (uint16_t) ((1.0 * sampleAverage / numSamples) + 0.5);
		
		lookUpTable[i] = sampleAverage;
	}
}


void LUTCalibratedADC::writeOnStream(Stream* stream) 
{
	stream->println("DACInput,ADCResult");

	uint16_t len = sizeof(lookUpTable) / sizeof(lookUpTable[0]);
	for (int i = 0; i < len; i++) 
	{
		stream->print(i * 16);
		stream->print(',');
		stream->println(lookUpTable[i]);
	}
}

void LUTCalibratedADC::saveOnFileCSV(File* file)
{	
	writeOnStream(file);
	file->close();	
}


