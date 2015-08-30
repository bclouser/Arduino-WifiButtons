
#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include "main.h"
#include "credentials.h"

#define SERVER_NAME "192.168.1.100"
#define SERVER_PORT (8888)

/* input lines for the buttons */
#define SHADES_BTN 14
#define LIGHTS_BTN 15
#define PIXELS_BTN 16
#define FAN_BTN 17

/* output lines for the pwm leds */
#define SHADES_LED 3
#define LIGHTS_LED 9
#define PIXELS_LED 10
#define FAN_LED 11

#define HEART_BEAT_LED 8
#define ESP_RST 2


/* global flag indicating the current connection state */
static bool socketActive = false;
// global flag to turn off the leds
static bool ledsOff = false;

SoftwareSerial debugSerial(12,13); // Rx, Tx
ESP esp(&Serial, &debugSerial, ESP_RST);
MQTT mqtt(&esp);
boolean wifiConnected = false;
boolean mqttConnection = false;

// Forward declaration
bool sendCmd(Cmd cmd);

void wifiCb(void* response)
{
    uint32_t status;
    RESPONSE res(response);

    if(res.getArgc() == 1) {
        res.popArgs((uint8_t*)&status, 4);
        if(status == STATION_GOT_IP) {
            debugSerial.println("WIFI CONNECTED");
            mqtt.connect(SERVER_NAME, SERVER_PORT, false);
            wifiConnected = true;
            //or mqtt.connect("host", 1883); /*without security ssl*/
        }
        else {
            wifiConnected = false;
            mqtt.disconnect();
        }
    }
}

void mqttConnected(void* response)
{
    debugSerial.println("mqtt Connected");
    mqttConnection = true;
}

void mqttDisconnected(void* response)
{
    debugSerial.println("mqtt connection was disconnected!");
    mqttConnection = false;
}

void mqttData(void* response)
{
  RESPONSE res(response);

  debugSerial.print("Received: topic=");
  String topic = res.popString();
  debugSerial.println(topic);

  debugSerial.print("data=");
  String data = res.popString();
  debugSerial.println(data);

}
void mqttPublished(void* response)
{
    RESPONSE res(response);
    debugSerial.println("publish callback: ");
    debugSerial.println(res.popString());
}


void setup(void)
{
    unsigned count = 0;
    // Set our buttons as inputs
    pinMode(SHADES_BTN, INPUT);
    pinMode(LIGHTS_BTN, INPUT);
    pinMode(PIXELS_BTN, INPUT);
    pinMode(FAN_BTN, INPUT);

    //Configure our PWM LED pins
    pinMode(SHADES_LED, OUTPUT);
    pinMode(LIGHTS_LED, OUTPUT);
    pinMode(PIXELS_LED, OUTPUT);
    pinMode(FAN_LED, OUTPUT);

    pinMode(HEART_BEAT_LED, OUTPUT);

    digitalWrite(HEART_BEAT_LED, HIGH);

    Serial.begin(19200);
    debugSerial.begin(19200);
    esp.enable();
    delay(500);
    esp.reset();
    delay(500);
    while(!esp.ready())
    {
        digitalWrite(HEART_BEAT_LED, HIGH);
        delay(1000);
        digitalWrite(HEART_BEAT_LED, LOW);
        delay(1000);
        if(count >= 3)
        {
            debugSerial.println("Resetting ESP chip");
            esp.enable();
            delay(500);
            esp.reset();
            delay(500);
            count = 0;
        }

        count++;
    }

    debugSerial.println("ARDUINO: setup mqtt client");
    if(!mqtt.begin("bclouse91@gmail.com", "", "", 120, 1)) {
        debugSerial.println("ARDUINO: fail to setup mqtt");
        // TODO, make some reasonable timeout and restart things
        while(1)
        {
            digitalWrite(HEART_BEAT_LED, HIGH);
            delay(5000);
            digitalWrite(HEART_BEAT_LED, LOW);
            delay(5000);
        }
    }


    debugSerial.println("ARDUINO: setup mqtt lwt");
    mqtt.lwt("/lwt", "offline", 0, 0); //or mqtt.lwt("/lwt", "offline");

    /*setup mqtt events */
    mqtt.connectedCb.attach(&mqttConnected);
    mqtt.disconnectedCb.attach(&mqttDisconnected);
    mqtt.publishedCb.attach(&mqttPublished);
    mqtt.dataCb.attach(&mqttData);

    /*setup wifi*/
    debugSerial.println("ARDUINO: setup wifi");
    esp.wifiCb.attach(&wifiCb);

    esp.wifiConnect(SSID,PASSWORD);

    debugSerial.println("ARDUINO: system started");
}
	
bool sendCmd(Cmd cmd)
{
	static const char* itemShades = "Curtains";
	static const char* itemLights = "Over Head Lights";
	static const char* itemPixels = "Pixel Wall";
	static const char* itemFan = "Fan";

	static uint8_t mux_id = 0;

    // figure out which itemName we are talking to
    const char* itemName = 0;
    switch(cmd)
    {
    	case e_cmdLights:
    		itemName = itemLights;
    		break;
    	case e_cmdShades:
    		itemName = itemShades;
    		break;
    	case e_cmdPixels:
    		itemName = itemPixels;
    		break;
    	case e_cmdFan:
    		itemName = itemFan;
    		break;
    	default:
    		debugSerial.println("Bad cmd passed to sendCmd()\n");
    		itemName = "";
    		return false;
    }

    char mesg[64] = {0};
    sprintf(mesg, "{\"command\":\"togglePower\",\"name\":\"%s\"}",itemName);


    debugSerial.println("This is what the message looks like: ");
    debugSerial.println(mesg);
    
    if( strlen(mesg) < 64 ) {
    	debugSerial.println("Value of strlen(mesg) is less than 64");
    }
    else {
    	debugSerial.println("Value of strlen(mesg) is more than 64");
    }

    // Send out command over mqtt protocol
    mqtt.publish("/bedroom", mesg);

    return true;
}

unsigned debounceDelay = 40;
int index = 0;
boolean ascending = true;
void loop(void)
{   
    digitalWrite(HEART_BEAT_LED, HIGH);
    // Throw some clock cycles at the esp
    esp.process();
    if(wifiConnected && mqttConnected)
    {
       // poll the buttons
    	if( digitalRead(SHADES_BTN) )
    	{
            ledsOff = !ledsOff;
    		debugSerial.println("Curtains button was pressed");
    		if( !sendCmd(e_cmdShades) )
    		{
    			debugSerial.println("Error sending e_cmdShades");
    		}
            delay(debounceDelay); //debounce delay
    	}
    	else if( digitalRead(LIGHTS_BTN) )
    	{
    		debugSerial.println("lights button was pressed");
    		if( !sendCmd(e_cmdLights) )
    		{
    			debugSerial.println("Error sending e_cmdLights");
    		}
            delay(debounceDelay); //debounce delay
    	}
    	else if( digitalRead(PIXELS_BTN) )
    	{
    		debugSerial.println("pixel wall button was pressed");
    		if( !sendCmd(e_cmdPixels) )
    		{
    			debugSerial.println("Error sending e_cmdPixels");
    		}
            delay(debounceDelay); //debounce delay
    	}
    	else if( digitalRead(FAN_BTN) )
    	{
    		debugSerial.println("Fan button was pressed");
    		if( !sendCmd(e_cmdFan) )
    		{
    			debugSerial.println("Error sending e_cmdFan");
    		}
            delay(debounceDelay); //debounce delay
    	}
    }

    analogWrite(SHADES_LED, index);
    analogWrite(LIGHTS_LED, index);
    analogWrite(PIXELS_LED, index);
    analogWrite(FAN_LED, index);

    digitalWrite(HEART_BEAT_LED, LOW);

    if(ascending) {
        if(index < 50)
        {
            index += 1;
        }
        else if(index <100)
        {
            index += 4;
        }
        else
        {
            index += 6;
        }
    }
    else {
        if(index > 50)
        {
            index -= 6;
        }
        else if(index >100)
        {
            index -= 4;
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

        analogWrite(SHADES_LED, 255);
        analogWrite(LIGHTS_LED, 255);
        analogWrite(PIXELS_LED, 255);
        analogWrite(FAN_LED, 255);
        delay(200);
    }

    // I wants to keep it off for a bit...
    // Makes it a bit more like breathing, so I feel less alone.
    if(index <= 0)
    {
        index = 0;
        ascending = true;

        analogWrite(SHADES_LED, 0);
        analogWrite(LIGHTS_LED, 0);
        analogWrite(PIXELS_LED, 0);
        analogWrite(FAN_LED, 0);
        delay(800);
    }

    // I don't care what you think or who you are. THE LEDS ARE OFF!
    if(ledsOff)
    {
        index = 0;
    }


    delay(40);
}
