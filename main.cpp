
#include <espduino.h>
#include <mqtt.h>
#include "main.h"
#include "credentials.h"
#include "MqttClient.h"
#include "Leds.h"


// global flag to turn off the leds
static bool ledsOff = false;

SoftwareSerial debugSerial(12,13); // Rx, Tx
ESP esp(&Serial, &debugSerial, ESP_RST);
MqttClient mqttClient(&esp);
volatile bool wifiConnected = false;

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
            wifiConnected = true;
        }
        else {
            debugSerial.println("WIFI failed to connect");
            wifiConnected = false;
        }
    }
}

void setup(void)
{
    // Set our buttons as inputs
    pinMode(SHADES_BTN, INPUT);
    pinMode(LIGHTS_BTN, INPUT);
    pinMode(PIXELS_BTN, INPUT);
    pinMode(FAN_BTN, INPUT);

    initLeds();

    Serial.begin(19200);
    debugSerial.begin(19200);

    esp.wifiCb.attach(&wifiCb);
    
    debugSerial.println("ARDUINO: Initial Setup Complete");
}

void espMultipleProcess(short numTimes)
{
    for(short processCount = 0; processCount<numTimes; ++processCount)
    {
        esp.process();
        delay(20);
    }
}
	
bool sendCmd(Cmd cmd)
{
	static const char* itemShades = "Curtains";
	static const char* itemLights = "Over Head Lights";
	static const char* itemPixels = "Pixel Wall";
	static const char* itemFan = "Fan";

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


    //debugSerial.println("This is what the message looks like: ");
    debugSerial.println(mesg);

    // Send out command over mqtt protocol
    mqttClient.publish("/bedroom", mesg);

    return true;
}

void bringUpWifi()
{
    unsigned triesBeforeChipReset = 2000;
    unsigned count = 0;
    while(!wifiConnected)
    {
        // Start by putting chip in known state
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


        /*setup wifi*/
        debugSerial.println("ARDUINO: Setting up wifi");
        esp.wifiConnect(SSID,PASSWORD);
        // We should chill here until we get a wifi connection
        while(!wifiConnected && (count < triesBeforeChipReset) )
        {
            esp.process();
            delay(100);
            ++count;
        }
        count = 0;
    }
}

void bringUpMqtt()
{
    unsigned brokerConnectionTryLimit = 500;
    unsigned count = 0;
    // if we lost wifiConnectivity, we need to break out and 
    // go back to bringing up wifi

    if( !mqttClient.init() )
    {
        debugSerial.println("ARDUINO: Bad news, mqtt client failed to initialize correctly");
        return;
    }

    while(!MqttClient::connected && (count < brokerConnectionTryLimit) )
    {
        debugSerial.println("Connecting mqttClient");
        mqttClient.connect(SERVER_NAME, SERVER_PORT, false);
        // process 10 times
        espMultipleProcess(10);
        ++count;
    }
    return;
}


const unsigned debounceDelay = 20;
int index = 0;
unsigned cyclesCount;
unsigned char buttonPressed = 0x00;
unsigned char lastButtonPressed = 0x00;
unsigned lastButtonPressExpireTimer = 0;
const unsigned lastButtonPressTimeout = 5000;
bool forceLedUpdate = true;

void loop(void)
{   

    bringUpWifi();

    if(wifiConnected)
    {
        bringUpMqtt();
    }

    if(wifiConnected && MqttClient::connected)
    {
        debugSerial.println("All systems online!");
    }
    

    while(wifiConnected && MqttClient::connected)
    {
        // Throw some clock cycles at the esp
        esp.process();

        buttonPressed = (PINC & 0x0F);
        
        if( buttonPressed )
        {
            // lastButtonPressed will be cleared after a time
            if( buttonPressed!=lastButtonPressed )
            {
            	if( buttonPressed == SHADES_BTN)
            	{
                    ledsOff = !ledsOff;
                    forceLedUpdate = true;
            		//if( !sendCmd(e_cmdShades) )
            		//{
            		//	debugSerial.println("Error sending e_cmdShades");
            		//}
            	}
            	else if( buttonPressed == LIGHTS_BTN )
            	{
            		if( !sendCmd(e_cmdLights) )
            		{
            			debugSerial.println("Error sending e_cmdLights");
            		}
            	}
            	else if( buttonPressed == PIXELS_BTN )
            	{
            		if( !sendCmd(e_cmdPixels) )
            		{
            			debugSerial.println("Error sending e_cmdPixels");
            		}
            	}
            	else if( buttonPressed == FAN_BTN )
            	{
            		if( !sendCmd(e_cmdFan) )
            		{
            			debugSerial.println("Error sending e_cmdFan");
            		}
            	}

                espMultipleProcess(5);
                delay(debounceDelay); //debounce delay
                // save lastButtonPressed to check against next time
                lastButtonPressed = buttonPressed;
                buttonPressed = 0x00;
            }
            else
            {
                // They pressed the same button again, restart the counter
                lastButtonPressExpireTimer = 0;
            }
        }
        if(lastButtonPressed)
        {
            ++lastButtonPressExpireTimer;
        }
        // After a while, we clear the lastButtonPressed
        // The point is to prevent a user holding a button
        if(lastButtonPressExpireTimer > lastButtonPressTimeout)
        {
            lastButtonPressed = 0x00;
            lastButtonPressExpireTimer = 0;
        }

        ++cyclesCount;

        if( (cyclesCount >= ledUpdateWaitCycles) || forceLedUpdate )
        {
            updateButtonLeds(ledsOff);
            cyclesCount = 0;
            forceLedUpdate = false;
        }
    }
}
