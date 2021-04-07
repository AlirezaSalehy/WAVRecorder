#include "AudioTimer.h"

#ifdef ESP32
hw_timer_t* timer = NULL;
#endif

void AudioTimer::setup(uint32_t frequency, void (*f)(void)) {
	stop();

#ifdef ESP8266
	timer1_attachInterrupt(f);	
	uint32_t tick = (80 * 1000000) / frequency;
	timer1_write(tick); /* ( 80 / 1 ) * 125 */
#endif

#ifdef ESP32
	timer = timerBegin(0, 20, true);
	timerAttachInterrupt(timer, f, true);
	timerAlarmWrite(timer, 4000000 / frequency, true); // 500 for 8Khz, 250 for 16Khz, 125 for 32Khz
#endif


#ifdef ARDUINO_SAM_DUE 
	// Here we are going to use timer 4
	Timer4.attachInterrupt(f).setFrequency(frequency);
#endif


}

void AudioTimer::start() {
#ifdef ESP8266
	timer1_enable(TIM_DIV1/*from 80 Mhz*/, TIM_EDGE/*TIM_LEVEL*/, TIM_LOOP/*TIM_SINGLE*/);
#endif

#ifdef ESP32
	timerAlarmEnable(timer);
#endif

#ifdef ARDUINO_SAM_DUE 
	Timer4.start();
#endif
}

void AudioTimer::stop() {
#ifdef ESP8266
	timer1_disable();
#endif

#ifdef ESP32
	if (timer)
		timerAlarmDisable(timer);
#endif

#ifdef ARDUINO_SAM_DUE 
	Timer4.stop();
#endif	
}