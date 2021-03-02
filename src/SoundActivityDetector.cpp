#include "SoundActivityDetector.h"

volatile bool isCalibrated = false;
uint16_t numSamples = 0;
bool isAverageSet = false;
bool isVectorSet = false;
uint32_t samplesCounter = 0;
uint32_t samplesAddition = 0;

static AudioTimer* timer = 0;
HardwareSerial* s = 0;

uint16_t averageVector = 0;
uint16_t averageValue = 0;

uint8_t ADCPin = 0;


#if defined(ESP8266) || defined(ESP32)
	void ICACHE_RAM_ATTR timerISR(void);
#elif
	void timerISR(void);
#endif 

// WAVRecorder buffer must be atleast equal to minimumLengthNS
SoundActivityDetector::SoundActivityDetector(uint8_t ADCPin_arg, uint16_t num_samples, uint16_t minimumLengthNS, uint16_t maximumRewardNS, HardwareSerial* serial_arg) {
	//audioChannels = channels;
	//numChannels = numChannels_arg;
	ADCPin = ADCPin_arg;
	
	this->minimumLengthNS = minimumLengthNS;
	this->maximumRewardNS = maximumRewardNS;
	setThresholds(3, 10);
	
	s = serial_arg;
	
	timer = new AudioTimer();
	
#ifdef ESP8266
 	wifi_set_opmode(NULL_MODE);
#endif

	calibrate(num_samples);
}

void SoundActivityDetector::calibrate(uint16_t num_samples) {
	numSamples = num_samples;
	timer->setup(8000, timerISR);
	timer->start();
	
	while (!isCalibrated) {}
	s->println("Done lets have fun!");
}

void SoundActivityDetector::calibrate(uint16_t averageVector, uint16_t averageValue) {
	averageValue = averageValue;
	averageVector = averageVector;
}

void SoundActivityDetector::setThresholds(uint8_t lower, uint8_t upper) {
	this->lower_threshold = lower;
	this->upper_threshold = upper;
}

// detect can be implemented in way that be called by Timer routine or the SD write function.
bool SoundActivityDetector::detect(uint16_t result16) {
	static uint32_t rewardCounter = 0, sampleCounter = 0;
	
	uint32_t vectorDeviation = abs(abs(result16 - averageValue) - averageVector);
	if (rewardCounter <= maximumRewardNS && vectorDeviation > this->upper_threshold)
		rewardCounter += 4;

	else if (vectorDeviation < this->lower_threshold) 
		if (rewardCounter >= 3)
		  	rewardCounter -= 3;
	else
		if (rewardCounter >= 1)
	  		rewardCounter -= 1;
	
	if (rewardCounter >= 30) 
		sampleCounter++;

	else
		sampleCounter = 0;		
	
	if (sampleCounter > minimumLengthNS)
		return true;		
		
	else
		return false;
}

// detect can be implemented in way that be called by Timer routine or the SD write function.
inline bool SoundActivityDetector::detect(uint16_t* buffer, uint16_t len) {
	static uint32_t rewardCounter = 0, sampleCounter = 0;
	
	for (size_t i = 0; i < len; i++) {
		uint32_t vectorDeviation = abs(abs(buffer[i] - averageValue) - averageVector);
		if (rewardCounter <= maximumRewardNS && vectorDeviation > this->upper_threshold)
			rewardCounter += 4;
	
		else if (vectorDeviation < this->lower_threshold) 
			if (rewardCounter >= 3)
			  	rewardCounter -= 3;
		else
			if (rewardCounter >= 1)
		  		rewardCounter -= 1;
		
		// 
		if (rewardCounter >= 30) 
			sampleCounter++;
	
		else
			sampleCounter = 0;		
		
		if (sampleCounter > minimumLengthNS)
			return true;		
	}
	
	return false;
}

uint16_t SoundActivityDetector::getMinimumLengthNS() {
	return this->minimumLengthNS;
}


 
#if defined(ESP8266) || defined(ESP32)
	void ICACHE_RAM_ATTR timerISR(void){
#elif
	void timerISR(void){
#endif 
	
	uint16_t result16;

#if defined(ARDUINO_SAM_DUE) || defined(ESP32)
	result16 = analogRead(ADCPin);
#endif

#if ESP8266
	system_adc_read_fast(&result16, 1, 8);
#endif
	
	if (!isAverageSet) 
		samplesAddition += result16;	
	else
		samplesAddition += abs(result16 - averageValue);
	
	samplesCounter++;
	if (samplesCounter >= numSamples) {
		if (!isAverageSet) {
			averageValue = samplesAddition / samplesCounter;	
			s->print("This is the average value: ");
			s->println(averageValue);
			isAverageSet = true;			
		}
		else if (!isVectorSet) {
			averageVector = samplesAddition / samplesCounter;
			s->print("This is the average vector: ");
			s->println(averageVector);
			isVectorSet = true;			

			isCalibrated = true;
			isAverageSet = false;
			isVectorSet = false;
			
			timer->stop();
		}

		samplesAddition = 0;
		samplesCounter = 0;
	}
}