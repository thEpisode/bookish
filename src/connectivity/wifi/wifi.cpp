/*
 * wifi.cpp
 *
 * Author: Camilo Rodriguez Cuaran
 * Company: Virtual capital of America
 * License: GNU General Public License v3.0
 * Created on: Jan 15, 2020
 * Updated on: Sep 09, 2022
 */

#include "Arduino.h"

#include "wifi.h"

Wifi::Wifi(String deviceId)
{
  _deviceId = deviceId;
}

String Wifi::connect(String ssid, String password)
{
  Serial.println("Connecting to " + ssid + " with password " + password);

  _ssid = ssid;
  _password = password;

  pinMode(2, OUTPUT);

  WiFi.setHostname(_deviceId.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  do
  {
    digitalWrite(2, HIGH);
    Serial.print(".");
    delay(500);
    digitalWrite(2, LOW);
  } while (WiFi.status() != WL_CONNECTED);

  digitalWrite(2, HIGH);

  String ipStr = "";
  IPAddress ip = WiFi.localIP();

  for (int i = 0; i < 4; i++)
  {
    ipStr += i ? "." + String(ip[i]) : String(ip[i]);
  }

  Serial.println("Local Ip: " + ipStr);
  return ipStr;
}

bool Wifi::isConnected()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    return false;
  }
  else if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }
}

void Wifi::enableAP(String ssid, String password, int localIp[], int gateway[], int subnet[])
{
  Serial.println("Enabling Access Point...");
  IPAddress local_IP(localIp[0], localIp[1], localIp[2], localIp[3]);
  IPAddress gateway_IP(gateway[0], gateway[1], gateway[2], gateway[3]);
  IPAddress subnet_IP(subnet[0], subnet[1], subnet[2], subnet[3]);

  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid.c_str(), password.c_str());
  delay(100);
  WiFi.softAPConfig(local_IP, gateway_IP, subnet_IP);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void Wifi::handleRoot()
{
  Serial.println("Handling root...");
  String message = "Welcome to this API\n\n";

  _server->send(200, "text/plain", message);
}

void Wifi::handleNotFound()
{
  Serial.println("Handling not found...");
  String message = "File Not Found\n\n";

  _server->send(404, "text/plain", message);
}

void Wifi::handleAPI(Lora *_loraController)
{
  Serial.println("Handling api...");
  String payload = _server->arg("payload");

  if (payload.length() <= 0)
  {
    return;
  }

  _server->send(200, "application/json", payload);

  if (_loraController->isConnected() == true)
  {
    Serial.println("Attempt to send message from LoRa...");
    _loraController->sendMessage(payload, 0xF3);
  }
}

void Wifi::enableServer(int port, Lora *_loraController)
{
  Serial.println("Enabling Server...");
  _server = new WebServer(port);
  _server->begin();

  _server->on("/", HTTP_GET, std::bind(&Wifi::handleRoot, this));
  _server->on("/insight", HTTP_POST, std::bind(&Wifi::handleAPI, this, _loraController));
  _server->onNotFound(std::bind(&Wifi::handleNotFound, this));
}

void Wifi::setStaticIp(int localIp[], int gateway[], int subnet[], int primaryDNS[], int secondaryDNS[])
{
  Serial.println("Enabling static ip...");
  IPAddress local_IP(localIp[0], localIp[1], localIp[2], localIp[3]);
  IPAddress gateway_IP(gateway[0], gateway[1], gateway[2], gateway[3]);
  IPAddress subnet_IP(subnet[0], subnet[1], subnet[2], subnet[3]);
  IPAddress primaryDNS_IP(primaryDNS[0], primaryDNS[1], primaryDNS[2], primaryDNS[3]);
  IPAddress secondaryDNS_IP(secondaryDNS[0], secondaryDNS[1], secondaryDNS[2], secondaryDNS[3]);

  if (!WiFi.config(local_IP, gateway_IP, subnet_IP, primaryDNS_IP, secondaryDNS_IP))
  {
    Serial.println("STA Failed to configure");
  }
  else
  {
    Serial.println("Static IP configured...");
  }
}

void Wifi::sendMessage(String origin, String endpoint, String payload, String method)
{
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(origin.c_str(), httpPort))
  {
    Serial.println("connection failed");
    return;
  }

  Serial.println("Requesting POST to " + origin + endpoint);
  Serial.println("Payload: " + payload);
  // Send request to the server:
  client.println(method + " " + endpoint + " HTTP/1.1");
  client.println("Host: " + origin);
  client.println("User-Agent: ESP32/1.0");
  client.println("Accept: */*");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(payload.length());
  client.println();
  client.print("payload=");
  client.print(payload);

  if (client.connected())
  {
    client.stop();
  }

  Serial.println("Request sent");
  delay(1000);

  while (client.available())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);

    // I will neeed to filter the headers, but first I want to be able to write to TempGraph anything
  }
  /* HTTPClient http;

  http.begin(endpoint);
  http.addHeader("Content-Type", "multipart/form-data");

  int httpCode = http.POST("Content-Disposition: form-data; name=\"payload\"\n" + payload); //Send the request
  String response = http.getString();

  Serial.println(endpoint + ": " + httpCode);
  Serial.println(response); //Print request response payload
  Serial.println("===================");

  http.end(); //Close connection */
}

void Wifi::APServerClientHandling()
{
  return _server->handleClient();
}