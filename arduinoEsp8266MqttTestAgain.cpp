#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include <avr/power.h>

// SSID of WiFi network
#define SSID "bittner"
// Password of WiFi network
#define PASS "S3m1c0l0n&C0"
// Reset pin
#define ESP_RST         4

#define MQTT_SERVER     "test.mosquitto.org"
#define MQTT_PORT       1883

#define SERVER_1_NAME     "Test Server 1"
#define SERVER_1_LED_NAME "Server 1"
#define SERVER_1_TOPIC    "/SomeTopicRoot/Topic1"

#define SERVER_2_NAME     "Test Server 2"
#define SERVER_2_LED_NAME "Server 2"
#define SERVER_2_TOPIC    "/SomeTopicRoot/Topic2"

#define JENKINS_SUCCESS "SUCCESS"

#define DEBUG_RX 11
#define DEBUG_TX 12
#define DEBUG_BAUD 19200

#define ESP8266_BAUD 19200

#define WIFI_GIVE_UP 30000 // ms

#define LED_PIN            13
#define SCROLL_SPEED 100 // ms

// Struct to describe a Jenkins instance
typedef struct JENKINS_SERVER
{
    const String name;
    const String ledName;
    const String topic;

};

const JENKINS_SERVER SERVERS[] =
{
    {
        SERVER_1_NAME,
        SERVER_1_LED_NAME,
        SERVER_1_TOPIC
    },
    {
        SERVER_2_NAME,
        SERVER_2_LED_NAME,
        SERVER_2_TOPIC
    }

};

// Debug interface
SoftwareSerial debug(DEBUG_RX, DEBUG_TX);
ESP esp(&Serial, &debug, ESP_RST);
MQTT mqtt(&esp);

unsigned int timeOfWiFiConnect;
unsigned int lastScrollUpdate;
boolean wifiConnected = false;
boolean mqttClientConnected = false;
unsigned int mqttAckIndex = 0;
unsigned int mqttAcking = false;

// Value that will be pushed to the shift register for the LEDs
boolean connecting = false;
boolean statuses[2];
int statusIndex;

void setup()
{
    timeOfWiFiConnect = 0;
    wifiConnected = false;
    mqttClientConnected = false;
    mqttAckIndex = 0;
    mqttAcking = false;

    // Initial setup for Arduino and attached hardware.
    debug.begin(DEBUG_BAUD);
    debug.println(F("Setup started"));

    // Configure the ESP8266, the MQTT client and the WiFi connection.
    initESP8266();

    debug.println(F("Setup finished"));
}

void initESP8266()
{
    debug.println(F("initESP8266"));
    Serial.begin(19200);

    debug.println(F("Enabling ESP8266"));
    esp.enable();
    delay(500);

    resetESP8266();

    debug.println(F("Waiting for ESP8266 to become ready..."));
    while(!esp.ready());
    debug.println(F("ESP8266 is ready"));

    debug.println(F("Asking MQTT client to begin..."));
    // NOTE: Replace with your email address.
    if(!mqtt.begin("bclouse91@gmail.com", "", "", 120, 1))
    {
        debug.println(F("MQTT client begin FAILED."));
        resetArduino();
        while(1);
    }

    // Set up Last Will and Testament (probably not required)
    debug.println(F("Setting last will and testament"));
    mqtt.lwt("/lwt", "offline", 0, 0);

    // Set MQTT callbacks.
    debug.println(F("Attaching MQTT callbacks"));
    mqtt.connectedCb.attach(&mqttConnected);
    mqtt.disconnectedCb.attach(&mqttDisconnected);
    mqtt.publishedCb.attach(&mqttPublished);
    mqtt.dataCb.attach(&mqttData);

    // Set the callback for when the WiFi is connected.
    debug.println(F("Attaching WiFi connection callback"));
    esp.wifiCb.attach(&wifiCb);

    // Connect to WiFi and wait for callback.
    debug.println(F("WiFi connecting..."));
    esp.wifiConnect(SSID, PASS);
}

void wifiCb(void* response)
{
    debug.println(F("WiFi callback received"));
    unsigned int status;
    RESPONSE res(response);

    if(res.getArgc() == 1)
    {
        res.popArgs((uint8_t*)&status, 4);
        if(status == STATION_GOT_IP)
        {
            debug.println(F("WiFi connected!"));
            debug.println(F("MQTT client connecting..."));
            mqtt.connect(MQTT_SERVER, MQTT_PORT, false);
            wifiConnected = true;
        }
        else
        {
            debug.println(F("WiFi connection FAILED"));
            wifiConnected = false;
            mqtt.disconnect();
        }
    }
}

void loop()
{
    esp.process();
    int notConnected = 0;

    if (wifiConnected)
    {
        notConnected = 0;
    }
    else
    {
        notConnected++;
        if ((notConnected % 100) == 0)
        {
            debug.print(F("WiFi still not connected: "));
            debug.print("some amount of time");
            debug.println(F(" ms"));
            notConnected = 0;
        }
    }

    /*if (!wifiConnected && (elapsed > WIFI_GIVE_UP))
    {
        elapsed = 0;
        resetArduino();
        return;
    }*/


    int serverCount = sizeof(SERVERS) / sizeof(JENKINS_SERVER);
    if (mqttClientConnected && !mqttAcking && (mqttAckIndex < serverCount))
    {
        ackNextSubscribe();
    }

    delay(30000);
    ackNextSubscribe();
}

void ackNextSubscribe()
{
    mqttAcking = true;
    String topicStr = SERVERS[mqttAckIndex].topic;
    topicStr += "/1";

    int length = topicStr.length() + 1;
    char topic[length];
    SERVERS[mqttAckIndex].topic.toCharArray(topic, length);

    debug.print(F("Subscribing to: "));
    debug.println(topicStr);

    mqtt.subscribe("/SomeTopicRoot/Topic/1");
}

void mqttConnected(void* response)
{
    debug.println(F("Connected!"));
    mqttClientConnected = true;
}

void mqttDisconnected(void* response)
{
    debug.println(F("Disconnected."));
    resetArduino();
}

void mqttData(void* response)
{
    debug.println(F("Data received"));

    if (mqttAcking)
    {
        mqttAcking = false;
        mqttAckIndex++;
    }

    RESPONSE res(response);

    debug.print(F("Received: topic="));
    String topic = res.popString();
    debug.println(topic);

    debug.print(F("data="));
    String data = res.popString();
    debug.println(data);
}

void mqttPublished(void* response)
{
    debug.println(F("Published"));
}

void resetESP8266()
{
    debug.println(F("Resetting ESP8266..."));
    debug.flush();

    esp.reset();
    delay(5000);  //ESP8266 takes a while to restart
}

void resetArduino()
{
    debug.println(F("Resetting Arduino..."));
    delay(1000);
    setup();
}