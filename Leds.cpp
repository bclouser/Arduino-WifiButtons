#include <Arduino.h>
#include "Leds.h"


unsigned ledUpdateWaitCycles = DEFAULT_LED_WAIT_CYLES;

void initLeds()
{

	//Configure our PWM LED pins
	pinMode(SHADES_LED, OUTPUT);
	pinMode(LIGHTS_LED, OUTPUT);
	pinMode(PIXELS_LED, OUTPUT);
	pinMode(FAN_LED, OUTPUT);

	pinMode(HEART_BEAT_LED, OUTPUT);

	// start on high
	PORTB |= HEART_BEAT_LED;
}


void toggleHeartBeatLed()
{
	PORTB = (PORTB & 0xFE) | (~(PORTB & 0X01));
}

void updateButtonLeds(bool ledsOff)
{
	static bool ascending = true;
	static int index = 0;

    if(ledsOff)
    {
        analogWrite(SHADES_LED, 0);
	    analogWrite(LIGHTS_LED, 0);
	    analogWrite(PIXELS_LED, 0);
	    analogWrite(FAN_LED, 0);
	    return;
    }

    if(ascending) {
        if(index < 50)
        {
            index += 1;
        }
        else if(index <100)
        {
            index += 2;
        }
        else
        {
            index += 5;
        }
    }
    else {
        if(index > 100)
        {
            index -= 5;
        }
        else if(index >50)
        {
            index -= 2;
        }
        else
        {
            index -= 1;
        }
    }

    if(index >= 255)
    {
        index = 255;
        ascending = false; 
        ledUpdateWaitCycles = DEFAULT_LED_WAIT_CYLES * 60;
    }

    // I wants to keep it off for a bit...
    // Makes it a bit more like breathing, so I feel less alone.
    else if(index <= 0)
    {
        index = 0;
        ascending = true;
        ledUpdateWaitCycles = DEFAULT_LED_WAIT_CYLES * 50;
    }
    else
    {
    	ledUpdateWaitCycles = DEFAULT_LED_WAIT_CYLES;
    }

    analogWrite(SHADES_LED, index);
    analogWrite(LIGHTS_LED, index);
    analogWrite(PIXELS_LED, index);
    analogWrite(FAN_LED, index);
 	
}