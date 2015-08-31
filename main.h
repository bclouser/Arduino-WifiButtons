#ifndef MAIN_H
#define MAIN_H

#include <SoftwareSerial.h>

#define SERVER_NAME "192.168.1.101"
#define SERVER_PORT (1883)

/* input lines for the buttons PORTC*/
#define SHADES_BTN 0x01
#define LIGHTS_BTN 0x02
#define PIXELS_BTN 0x04
#define FAN_BTN 0x08

#define ESP_RST 2

enum Cmd
{
	e_cmdShades,
	e_cmdLights,
	e_cmdPixels,
	e_cmdFan,
	e_cmdUndef
};

// externed so other modules can use it. I could make a singleton wrapper? hmmm.
extern SoftwareSerial debugSerial;

#endif