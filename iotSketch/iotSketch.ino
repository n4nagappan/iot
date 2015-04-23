/*
  Web client

 This sketch connects to a website 
 using Wi-Fi functionality on MediaTek LinkIt platform.

 Change the macro WIFI_AP, WIFI_PASSWORD, WIFI_AUTH and SITE_URL accordingly.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 modified 20 Aug 2014
 by MediaTek Inc.
 */

#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>

#define WIFI_AP "AndroidAP"
#define WIFI_PASSWORD "nuscfast"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define SITE_URL "www.722b4490.ngrok.io"

LWiFiClient c;

void setup()
{
  LTask.begin();
  LWiFi.begin();
  Serial.begin(115200);

  // keep retrying until connected to AP
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
    Serial.println("Trying again...");
  }
}

boolean disconnectedMsg = false;

void sendGpsData(){
  // keep retrying until connected to website
  Serial.println("Connecting to WebSite");
  while (0 == c.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }

  // send HTTP request, ends with 2 CR/LF
  Serial.println("send HTTP GET request");
  c.println("GET http://722b4490.ngrok.io/gps?lat=1.299155&long=103.787679 HTTP/1.1");
  c.println("Host: " SITE_URL);
  c.println("Connection: keep-alive");
  c.println();

  // waiting for server response
  Serial.println("waiting HTTP response:");
  while (!c.available())
  {
    delay(100);
  }
  
  // Read the data here
  // Make sure we are connected, and dump the response content to Serial
  while (c)
  {
    int v = c.read();
    if (v != -1)
    {
      Serial.print((char)v);
    }
    else
    {
      Serial.println("no more content, disconnect");
      c.stop();
    }
  }

  if (!disconnectedMsg)
  {
    Serial.println("disconnected by server");
    disconnectedMsg = true;
  }
}

void loop()
{
  // every 5 seconds
  sendGpsData();
  delay(5000);
}

