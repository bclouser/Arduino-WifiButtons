# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt

mqttc = mqtt.Client("python_pub")
mqttc.connect("192.168.1.101", 1883)
mqttc.publish("/bensRoom/Shades", "1")
mqttc.loop(2) #timeout = 2s
