#include "wifi_helper.h"
#include <Arduino.h>

#define ST(A) #A
#define STR(A) ST(A)

#define SaveDisconnectTime 1000 // Time im ms for save disconnection, needed to avoid that WiFi works only evey secon boot: https://github.com/espressif/arduino-esp32/issues/2501

void wifi_setup() {
  wifi_ensure_connected();
  ArduinoOTA.setHostname(STR(HOSTNAME));
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void wifi_loop() {
  wifi_ensure_connected();
  ArduinoOTA.handle();
}

void wifi_ensure_connected() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  digitalWrite(LED_BUILTIN, true);
  wifi_connect_strongest();
  digitalWrite(LED_BUILTIN, false);
}

void wifi_connect_strongest() {
  // Clear all of the cached wifi data
  // avoid the following bug in ESP32 WiFi connection: https://github.com/espressif/arduino-esp32/issues/2501
  WiFi.disconnect(true); // delete old config
  WiFi.persistent(false); //Avoid to store Wifi configuration in Flash
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true); // delete old config
  WiFi.begin();
  delay(SaveDisconnectTime); // 500ms seems to work in most cases, may depend on AP
  WiFi.disconnect(true); // delete old config

  Serial.println("scan start");
  int n = WiFi.scanNetworks(); // WiFi.scanNetworks will return the number of networks found
  Serial.println("scan done");

  uint8_t bssidIdx = 0;
  uint8_t *bssid = (uint8_t *)__null;
  int32_t rssi = -500;

  for (int i = 0; i < n; i++) {
    if (String(WiFi.SSID(i)) == String(ssid)) {
      if (WiFi.RSSI(i) > rssi) {
        bssidIdx = i;
        bssid = WiFi.BSSID(i);
        rssi = WiFi.RSSI(i);
      }
    }
  }

  if (bssid != __null) {
    Serial.print("Using BSSID: ");
    Serial.println(WiFi.BSSIDstr(bssidIdx));
  }

  WiFi.begin(ssid, passphrase, 0, bssid, true);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("Connection successful");
}
