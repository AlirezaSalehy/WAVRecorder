#include "AudioSystem.h"

AudioSystem::AudioSystem() {
	audioLogger = &Serial;  
	source = new AudioFileSourceSD();

	#ifdef ESP8266
		output = new AudioOutputI2S(SPDIF_OUT_PIN);
	#elif ESP32
		output = new AudioOutputI2S(0, INTERNAL_DAC);
	#endif	
}

AudioSystem::AudioSystem(uint8_t csPin) : AudioSystem::AudioSystem() {
	//if (!SD.begin(csPin)) {
	//	Serial.println("SD initialization failed");
	//}
}

void AudioSystem::playAudioBlocking(char *fileName) {
	bool toReturn = false;
	
	if (decoder && decoder->isRunning()) {
    	decoder->stop();
  	}
	
	
	File audioFile = SD.open(fileName, FILE_READ);
	if (!audioFile) {
		Serial.println("File can not be read!");
		toReturn = true;
	}
	
	if (String(audioFile.name()).endsWith(".wav")) {
		decoder = new AudioGeneratorWAV();	
	}
	else if (String(audioFile.name()).endsWith(".mp3")) {
		decoder = new AudioGeneratorMP3();
	}
	else {
		Serial.println("File does not end with .wav or .mp3");
		toReturn = true;
	}
	
	if (toReturn == true) {
		return;
	}
	
	source->close();
	if (source->open(audioFile.name())) {
		decoder->begin(source, output);
		if (!decoder->isRunning()) {
			Serial.println("Decoder is not running");
		}
		else {
			Serial.printf_P(PSTR("Playing in from the file %s\n"), audioFile.name());
		}
		
		while (decoder->loop()) {}	
		
		decoder->stop();	
	}
	else {
		Serial.println("Error playing file");
	}

}

void AudioSystem::playAudioLoop() {
	decoder->loop();
}
