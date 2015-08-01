/**
 * \file
 *       ESP8266 RESTful Bridge example
 * \author
 *       Tuan PM <tuanpm@live.com>
 */

#include <SoftwareSerial.h>
#include <espduino.h>
#include <rest.h>

SoftwareSerial debugPort(11, 12); // RX, TX

ESP esp(&Serial, &debugPort, 4);

REST rest(&esp);

boolean wifiConnected = false;

void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {
      debugPort.println("WIFI CONNECTED");

      wifiConnected = true;
    } else {
      wifiConnected = false;
    }

  }
}

void setup() {
  Serial.begin(19200);
  debugPort.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready());

  debugPort.println("ARDUINO: setup rest client");
  if(!rest.begin("http://www.multiplewanda.com")) {
      debugPort.println("ARDUINO: failed to setup rest client");
      while(1);
  }
  rest.setContentType("application/json");

  /*setup wifi*/
  debugPort.println("ARDUINO: setup wifi");
  esp.wifiCb.attach(&wifiCb);
  esp.wifiConnect("bittner","S3m1c0l0n&C0");
  debugPort.println("ARDUINO: system started");
}

void loop() {
  char data_buf[256] = {0};
  esp.process();
  if(wifiConnected) {
    //sprintf(data_buf, "{\"command\":\"getPowerControl\"}");
    //debugPort.println(data_buf);
    //rest.get("myDash/php/restApi/api_get.php?action=get_app_list");

    sprintf(data_buf, "action=get_app_list&param2=value2");
    rest.post("/myDash/php/restApi/api.php", (const char*)data_buf);
    debugPort.println("ARDUINO: send request");

    if(rest.getResponse(data_buf, 256) == HTTP_STATUS_OK){
        debugPort.println("ARDUINO: request successful");
        debugPort.print(data_buf);
    } else {
      debugPort.println("ARDUINO: POST error");
      debugPort.print(data_buf);
    }
    delay(2000);
  }
}