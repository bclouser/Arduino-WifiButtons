#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include "ITEADLIB_Arduino_WeeESP8266/ESP8266.h"


const int8_t numSquares = 96;
const int16_t maxBrightness = 0xFFF;

int main(void) {
	
	// Watch out. Infinite Loops
	//fadeTilesRunLoop(numSquares, maxBrightness);
	DDRB |= _BV(PB0) | _BV(PB1);

	PORTB |= _BV(PB0);
	while(1<2)
	{56
		PORTB |= _BV(PB0);
		PORTB |= _BV(PB1);
		_delay_ms(100);
		PORTB &= ~_BV(PB0);
		PORTB &= ~_BV(PB1);
		_delay_ms(100);
	}
}
