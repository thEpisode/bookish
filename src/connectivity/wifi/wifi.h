/*
 * wifi.h
 *
 * Author: Camilo Rodriguez Cuaran
 * Company: Virtual capital of America
 * License: GNU General Public License v3.0
 * Created on: Jan 15, 2020
 * Updated on: Sep 09, 2022
 */

#pragma once

#ifndef WIFI_H
#define WIFI_H

#include "Arduino.h"
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>

#include "../lora/lora.h"

typedef struct wifi_credentials
{
  char *ssid;
  char *pass;
};

class Wifi
{
public:
  Wifi(String deviceId);
  String connect(String ssid, String password);
  bool isConnected();
  void enableAP(String ssid, String password, int localIp[], int gateway[], int subnet[]);
  void enableServer(int port, Lora *_loraController);
  void setStaticIp(int localIp[], int gateway[], int subnet[], int primaryDNS[], int secondaryDNS[]);
  void sendMessage(String origin, String endpoint, String payload, String method);
  void APServerClientHandling();

private:
  String _deviceId;
  String _ssid;
  String _password;
  WebServer *_server;
  void handleRoot();
  void handleAPI(Lora *_loraController);
  void handleNotFound();
};

#endif