
#include <espduino.h>
#include <mqtt.h>
#include "main.h"
#include "credentials.h"
#include "MqttClient.h"
#include "Leds.h"


// global flag to turn off the leds
static bool ledsOff = false;

SoftwareSerial debugSerial(12,13); // Rx, Tx
ESP esp(&Serial, &debugSerial, ESP_CHIP_ENABLE);
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
            debugSerial.println("ARDUINO: WIFI CONNECTED");
            wifiConnected = true;
        }
        else {
            debugSerial.println("ARDUINO: WIFI failed to connect");
            wifiConnected = false;
            MqttClient::connected = false;
        }
    }
    else
    {
        debugSerial.println("ARDUINO: Wifi is doing something unusual...");
    }
}


void setup(void)
{

    // Set button pins (14-17) as inputs. Bits(0-3)
    DDRC &= 0xF0;

    // Set reset pin as output
    pinMode(ESP_CHIP_RESET, OUTPUT);
    // Keep reset pin held high for regular operation
    digitalWrite(ESP_CHIP_RESET, HIGH);

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

void hardResetEspChip()
{
    digitalWrite(ESP_CHIP_RESET, LOW);
    delay(20);
    digitalWrite(ESP_CHIP_RESET, HIGH);
    delay(20);
}
	
bool sendCmd(Cmd cmd)
{
	static const char* itemCurtains = "curtains";
	static const char* itemLights = "overHeadLights";
	static const char* itemPixels = "pixelWall";
	static const char* itemFan = "fan";

    // figure out which itemName we are talking to
    const char* itemName = 0;
    switch(cmd)
    {
    	case e_cmdButton1:
            itemName = itemCurtains;
    		break;
    	case e_cmdButton2:
            itemName = itemLights;
    		break;
    	case e_cmdButton3:
    		itemName = itemPixels;
    		break;
    	case e_cmdButton4:
    		itemName = itemFan;
    		break;
    	default:
    		debugSerial.println("ARDUINO: Bad cmd passed to sendCmd()\n");
    		itemName = "";
    		return false;
    }
    char topic[32] = {0};
    sprintf(topic, "/%s/%s", "bedroom1", itemName);

    char mesg[64] = {0};
    sprintf(mesg, "{\"command\":\"toggle\"}");
    debugSerial.println("");
    debugSerial.println(topic);
    debugSerial.println(mesg);

    // Send out command over mqtt protocol
    mqttClient.publish(topic, mesg);

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
        hardResetEspChip();
        delay(1000);
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
                debugSerial.println("ARDUINO: Resetting ESP chip");
                esp.enable();
                delay(500);
                hardResetEspChip();
                delay(1000);
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
    unsigned brokerConnectionTryLimit = 10;
    unsigned count = 0;
    // if we lost wifiConnectivity, we need to break out and 
    // go back to bringing up wifi
    debugSerial.println("ARDUINO: Inside bringUpMqtt()");


    if( !mqttClient.init() )
    {
        debugSerial.println("ARDUINO: Bad news, mqtt client failed to initialize correctly");
        return;
    }

    while(!MqttClient::connected && (count < brokerConnectionTryLimit) )
    {
        debugSerial.println("ARDUINO: Connecting mqttClient");
        mqttClient.connect(SERVER_NAME, SERVER_PORT, false);
        // process 10 times
        espMultipleProcess(10);
        delay(100);
        espMultipleProcess(10);

        ++count;
    }
    debugSerial.println("ARDUINO: Leaving bringUpMqtt()");

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

    if(!wifiConnected)
    {
        debugSerial.println("ARDUINO: Wifi down, bringing it up");
        bringUpWifi();
        
        espMultipleProcess(10);
        delay(1000);

    }

    if(!MqttClient::connected && wifiConnected)
    {
        debugSerial.println("ARDUINO: Mqtt down, bringing it up");
        bringUpMqtt();
        espMultipleProcess(10);
        delay(1000);
    }

    if(MqttClient::connected && !wifiConnected)
    {
        debugSerial.println("ARDUINO: Bad state. mqtt somehow connected and wifi isn't");
        debugSerial.println("ARDUINO: restarting state machine");
        wifiConnected = false;
        delay(1000);
    }

    // subscriptions!!!!
    if(wifiConnected && MqttClient::connected)
    {
        debugSerial.println("ARDUINO: All systems online!");
        // set subscriptions
        mqttClient.subscribe();
        espMultipleProcess(10);
        delay(1000);
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
            	if( buttonPressed == BTN_1)
            	{
                    ledsOff = !ledsOff;
                    forceLedUpdate = true;
                    debugSerial.println("Turning Button Leds");
                    debugSerial.println(ledsOff?"OFF":"ON");

            		//if( !sendCmd(e_cmdButton1) )
            		//{
            		//	debugSerial.println("Error sending e_cmdButton1");
            		//}
            	}
            	else if( buttonPressed == BTN_2 )
            	{
            		if( !sendCmd(e_cmdButton2) )
            		{
            			debugSerial.println("ARDUINO: Error sending e_cmdButton2");
            		}
            	}
            	else if( buttonPressed == BTN_3 )
            	{
            		if( !sendCmd(e_cmdButton3) )
            		{
            			debugSerial.println("ARDUINO: Error sending e_cmdButton3");
            		}
            	}
            	else if( buttonPressed == BTN_4 )
            	{
            		if( !sendCmd(e_cmdButton4) )
            		{
            			debugSerial.println("ARDUINO: Error sending e_cmdButton4");
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
