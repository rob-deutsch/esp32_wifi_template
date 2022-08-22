#include <Arduino.h>
#include "wifi_helper.h"

void setup(void) {
  Serial.begin(115200);
  wifi_setup();
}

void loop(void) {
  wifi_loop();
}