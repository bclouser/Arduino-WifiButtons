#ifndef LEDS_H
#define LEDS_H

/* output lines for the pwm leds */
#define SHADES_LED 3
#define LIGHTS_LED 9
#define PIXELS_LED 10
#define FAN_LED 11

#define HEART_BEAT_LED 0x01 /* PORTB */

#define DEFAULT_LED_WAIT_CYLES 4300


extern unsigned ledUpdateWaitCycles;


void initLeds();

void toggleHeartBeatLed();

void updateButtonLeds(bool ledsOff=false);


#endif