#include "WAVRecorder.h"

HardwareSerial* serial = 0;
uint32_t samplingRate = 0;
volatile uint32_t counter = 0;

WAVGenerator* wg = 0;

SoundActivityDetector* sad = 0;

/*
uint16_t soundBuffer[256];
volatile uint8_t lastIndex = 0;
*/

volatile uint8_t isStopped = 0;
volatile uint8_t isStarted = 0;

channel_t* audioChannels = 0;
uint16_t numChannels = 0;

uint8_t ADCDataLen = 0;
uint8_t devideFactor = 0;
uint8_t sampleLength = 0;

void 
#if defined(ESP8266) || defined(ESP32)
	ICACHE_RAM_ATTR 
#endif 
	sampleAndBuffer(){
	uint8_t result[numChannels];
	
	if (wg->isBufferFull()) {
		serial->println("Stop");
		return;
	}

	for (int i = 0; i < numChannels; i++)
	{
		uint16_t result16;
	#ifdef ESP8266
	    //system_soft_wdt_stop();
	    //ets_intr_lock( ); //close interrupt
	    //noInterrupts();
	
	    system_adc_read_fast(&result16, 1, 8);
	
	    //interrupts();
	    //ets_intr_unlock(); //open interrupt
	    //system_soft_wdt_restart();
	
	#elif defined(ARDUINO_SAM_DUE) || defined(ESP32)  
		result16 = analogRead(audioChannels[i].ADCPin); //averageADCSample(audioChannels[i].ADCPin, 1);
	#endif

		result[i] = ((result16 + 1) / devideFactor) - 1;
	
	}

	wg->appendBuffer(result, numChannels * 1, 1);
		
	counter++;
	if (counter % samplingRate == 0) {
		serial->println("1");
	}
}

void 
#if defined(ESP8266) || defined(ESP32)
	ICACHE_RAM_ATTR 
#endif 
	sampleAndBufferAutoDetection(){
	static uint8_t recordingStarted = 0;
	
	if (wg->isBufferFull()) {
		serial->println("Stop");
		return;
	}

	uint16_t result16 = 0;
#ifdef ESP8266
    system_adc_read_fast(&result16, 1, 8);
#endif

#if defined(ARDUINO_SAM_DUE) || defined(ESP32)  
	result16 = analogRead(audioChannels[0].ADCPin);
#endif
	
	uint8_t stat = sad->detect(result16);
	if (!recordingStarted && stat) {
		recordingStarted = 1;
		wg->markIgnoredChunks(sad->getMinimumLengthNS());
		isStarted = 1;
	} else if (recordingStarted && !stat) {
		recordingStarted = 0;
		isStopped = 1;
	}
	
	uint8_t result = ((result16 + 1) / devideFactor) - 1;
	wg->appendBuffer(&result, 1, stat);
		
	counter++;
	if (counter % samplingRate == 0) {
		serial->println("1");
	}
}

WAVRecorder::WAVRecorder(uint8_t ADCDataLen_arg, channel_t* channels_arg, uint16_t numChannels_arg, uint32_t samplingRate_arg, uint16_t sampleLength_arg, HardwareSerial* serial_arg) {
	audioChannels = channels_arg;
	numChannels = numChannels_arg;
	ADCDataLen = ADCDataLen_arg;
	serial = serial_arg;
	samplingRate = samplingRate_arg;
	sampleLength = sampleLength_arg;
	counter = 0;
	
	devideFactor = pow(2, ADCDataLen / sampleLength_arg);
}

WAVRecorder::WAVRecorder(uint8_t ADCDatLen_arg, channel_t* channels, uint16_t numChannels, File* dataFile, uint32_t samplingRate_arg, uint16_t sampleLength, HardwareSerial* serial_arg) 
: WAVRecorder(ADCDatLen_arg, channels, numChannels, samplingRate_arg, sampleLength, serial_arg)
{	
	wg = new WAVGenerator({dataFile, samplingRate, sampleLength, numChannels} , serial);

	// The code bellow is moved to the start()
	//timer = new AudioTimer();
	//timer->setup(sampleRate, sampleAndBuffer);
}

void WAVRecorder::setFile(File* dataFile) {
	counter = 0;
	isStarted = 0;
	isStopped = 0;

	if (wg)
		wg->setAudio({dataFile, samplingRate, sampleLength, numChannels});
	else
		wg = new WAVGenerator({dataFile, samplingRate, sampleLength, numChannels} , serial);
}

// Not yet implemented, but if we want it to be non blocking we should put the end clause in the 
// ISR of the timer
void WAVRecorder::start(uint32_t time) {

}

void WAVRecorder::start(SoundActivityDetector* sad_arg) {
	sad = sad_arg;
	timer = new AudioTimer();
	timer->setup(samplingRate, sampleAndBufferAutoDetection);
	timer->start();
}

void WAVRecorder::start() {
	timer = new AudioTimer();
	timer->setup(samplingRate, sampleAndBuffer);
	

	timer->start();
}

void WAVRecorder::stop() {
	timer->stop();
	wg->create();
}

void WAVRecorder::startBlocking(uint32_t time_ms) {
	start();
	
	while (counter <= ((time_ms / 1000) * samplingRate)) {
		wg->writeChunks();
	}
	
	stop();
}


void WAVRecorder::startBlocking(SoundActivityDetector* sad_arg) {
	start(sad_arg);
	
	digitalWrite(2, 1);
	while (!isStopped) {
		wg->writeChunks();
		
		if (isStarted)
			digitalWrite(2, 0);
		
	    yield();
	}
	digitalWrite(2, 1);
	
	stop();
}