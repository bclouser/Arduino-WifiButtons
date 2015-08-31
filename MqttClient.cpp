#include "MqttClient.h"

bool MqttClient::connected = false;


bool MqttClient::init()
{
	debugSerial.println("ARDUINO: setup mqtt client");

	if(!m_mqtt.begin("bclouse91@gmail.com", "", "", 120, 1)) 
	{
		return false;
	}

	debugSerial.println("ARDUINO: setup mqtt lwt");
	m_mqtt.lwt("/lwt", "offline", 0, 0); //or mqtt.lwt("/lwt", "offline");

	/*setup mqtt events */
	m_mqtt.connectedCb.attach(&MqttClient::onConnected);
	m_mqtt.disconnectedCb.attach(&MqttClient::onDisconnected);
	m_mqtt.publishedCb.attach(&MqttClient::onPublished);
	m_mqtt.dataCb.attach(&MqttClient::onData);
	return true;
}

void MqttClient::connect(const char* serverAddr, unsigned port, bool secure)
{
	m_mqtt.connect(serverAddr, port, secure);
}

void MqttClient::disconnect()
{
	m_mqtt.disconnect();
}

void MqttClient::publish(const char* location, char* mesg)
{
	m_mqtt.publish(location, mesg);
}


void MqttClient::onConnected(void* response)
{
    debugSerial.println("MQTT Connected!");
    connected = true;
}

void MqttClient::onDisconnected(void* response)
{
    debugSerial.println("MQTT connection was disconnected!");
    connected = false;
}

void MqttClient::onData(void* response)
{
  RESPONSE res(response);

  debugSerial.print("Received: topic=");
  String topic = res.popString();
  debugSerial.println(topic);

  debugSerial.print("data=");
  String data = res.popString();
  debugSerial.println(data);

}
void MqttClient::onPublished(void* response)
{
    //RESPONSE res(response);
    //debugSerial.println("publish callback: ");
    //debugSerial.println(res.popString());
}






