# WAVRecorder
Library for voice recording using Electret Microphones for ESP32, ESP8266 and Arduino Due.

The library has some components but the user interface component is <code>WAVRecorder</code> class, two methods of the class are implemented until now for recording, <code>startBlocking(uint32_t time_ms)</code> and <code>startBlocking(SoundActivityDetector* sad_arg)</code>.
<code>startBlocking(uint32_t time_ms)</code> records for specified amount of time in milliseconds. and the other one starts recording as soon as sound power exceeds specific threshold and stops recording as fall behind a specifi threshold.

you can use SD card to store recording file which is a WAV. file. or you can use external flash if you're using ESP32 or ESP8266. The library is capable of stereo recording which is only possible if using ESP32 and DUE because ESP8266 has only one ADC input pin.

You should specify channels before being able to record, each channels is defined by ADC pin number which connected to MIC output. no need to say that for stereo recording you need to define two channels.

There is other component in library to play recorded sound which is AudioSystem which is just a wrapper for cool [ESP8266Audio]https://github.com/earlephilhower/ESP8266Audio library.

# Example using SD card and a single microphone (single channel)
Below is a simple example for recording for 3000 ms and playing it back repeatedly. 
```cpp
#include "src/WAVRecorder.h"
#include "src/AudioSystem.h"

#define SAMPLE_RATE 16000
#define SAMPLE_LEN 8

// Hardware SPI's CS pin which is different in each board
#ifdef ESP8266 
  #define CS_PIN 16
#elif ARDUINO_SAM_DUE
  #define CS_PIN 4
#elif ESP32
  #define CS_PIN 5
#endif

// The analog pins (ADC inputs) which microphone outputs are connected to.
#define MIC_PIN_1 34
#define MIC_PIN_2 35

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


# Schematic (Only for recording)

![SD_Card_MIC_ESP32_Module](https://user-images.githubusercontent.com/49995349/113864691-7527f500-97c0-11eb-8b2c-52f717252f21.png)

![SD_Card_MIC_ESP32](https://user-images.githubusercontent.com/49995349/113864699-78bb7c00-97c0-11eb-98b8-7b3f08c90e56.png)
