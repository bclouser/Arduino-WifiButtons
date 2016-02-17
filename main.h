#ifndef MAIN_H
#define MAIN_H

#include <SoftwareSerial.h>

#define SERVER_NAME "192.168.1.200"
#define SERVER_PORT (1883)

/* input lines for the buttons PORTC (NOT ARDUINO DIGITAL NUMBERS)*/
#define BTN_1 0x01
#define BTN_2 0x04
#define BTN_3 0x02
#define BTN_4 0x08

#define ESP_CHIP_RESET 4
#define ESP_CHIP_ENABLE 2

enum Cmd
{
	e_cmdButton1,
	e_cmdButton2,
	e_cmdButton3,
	e_cmdButton4,
	e_cmdUndef
};

// externed so other modules can use it. I could make a singleton wrapper? hmmm.
extern SoftwareSerial debugSerial;

#endif