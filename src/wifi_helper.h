#pragma once

#include <WiFi.h>
#include <ArduinoOTA.h>
#include "wifi_credentials.h"

void wifi_setup();
void wifi_loop();
void wifi_ensure_connected();
void wifi_connect_strongest();
