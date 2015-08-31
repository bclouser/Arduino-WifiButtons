#ifndef MQTT_CLIENT
#define MQTT_CLIENT

#include <espduino.h>
#include <mqtt.h>
#include "main.h"


class MqttClient
{
public:
	MqttClient(ESP* esp):m_mqtt(esp){}
	~MqttClient(){}

	bool init();
	void connect(const char* serverAddr, unsigned port, bool secure=false);
	void disconnect();
	void publish(const char* location, char* mesg);
	static bool connected;

private:
	MQTT m_mqtt;
	static void onConnected(void *);
	static void onDisconnected(void *);
	static void onData(void *);
	static void onPublished(void *);
};

#endif



