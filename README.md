# WAVRecorder
Library for voice recording using Electret Microphones for ESP32, ESP8266 and Arduino Due.

The library has some components but the user interface component is <code>WAVRecorder</code> class, it has some methods 



# Example

```arduino
#include "src/WAVRecorder.h"
#include "src/AudioSystem.h"

#define SAMPLE_RATE 16000
#define SAMPLE_LEN 8

#ifdef ESP8266 
  #define CS_PIN 16
#elif ARDUINO_SAM_DUE
  #define CS_PIN 4
#elif ESP32
  #define CS_PIN 5
  #define MIC_PIN_1 34
  #define MIC_PIN_2 35
#endif

#define NUM_CHANNELS 1
channel_t channels[] = {{MIC_PIN_1}};

char file_name[] = "/sample.wav";
File dataFile;

//SoundActivityDetector* sadet;

void recordAndPlayBack();

void setup() {
  for (int i = 0; i < sizeof(channels)/sizeof(channel_t); i++)
    pinMode(channels[i].ADCPin, INPUT);
  //analogReadResolution(12); for ESP32
  
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println();

  // put your setup code here, to run once:
  if (!SD.begin(CS_PIN)) {
    Serial.println("Failes to initialize SD!");
  }
  else {
    Serial.println("SD opened successfuly");
  }
  SPI.setClockDivider(SPI_CLOCK_DIV2); // This is becuase feeding SD Card with more than 40 Mhz, leads to unstable operation. 
                                       // (Also depends on SD class) ESP8266 & ESP32 SPI clock with no division is 80 Mhz.

  as = new AudioSystem(CS_PIN);
  //sadet = new SoundActivityDetector(channels[0].ADCPin, 2000, 10 * 512, 6 * 512, &Serial);
  wr = new WAVRecorder(12, channels, NUM_CHANNELS, SAMPLE_RATE, SAMPLE_LEN, &Serial);

}

void loop() {
  // put your main code here, to run repeatedly:
  recordAndPlayBack();    
}

void recordAndPlayBack() {
    if (SD.exists(file_name)) {
      SD.remove(file_name);
      Serial.println("File removed!");
    }
   
    dataFile = SD.open(file_name, FILE_WRITE);
    if (!dataFile) {
      Serial.println("Failed to open the file!");  
      return;
    }
    
    // Setting file to store recodring 
    wr->setFile(&dataFile);

    Serial.println("Started");
    // With checks Sound power level and it exceeds a threshold recording starts and stops recording when power fall behind another threshold.
    //wr->startBlocking(sadet);
    
    // Recording for 3000 ms
    wr->startBlocking(3000);
    Serial.println("File Created");

    Serial.println("Playing file");
    as->playAudioBlocking(file_name); 
}
