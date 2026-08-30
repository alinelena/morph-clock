// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "config.h"
#include "mqtt.h"
#include "rgb_display.h"

WiFiClient wifiClient;
int status = WL_IDLE_STATUS;

// Initialize MQTT client
PubSubClient client(wifiClient);

// Time of last status update
unsigned long lastStatusSend = 0;

// Time of last client.loop()
unsigned long lastLoop = 0;
// Time of last weather sensor data receive
unsigned long lastSensorRead = 0;
// Time of last light sensor read
unsigned long lastLightRead = 0;

// NTP
const int8_t timeZone = 2;
const int8_t minutesTimeZone = 0;
bool wifiFirstConnected = false;
// Current time
struct tm timeinfo;

// Display
MatrixPanel_I2S_DMA *dma_display = nullptr;

// Flags to trigger display updates
bool clockStartingUp = true;
bool newSensorData = false;
bool sensorDead = true;

// Heartbeat marker
bool heartBeat = true;

// Log message persistence
// Is a log message currently displayed?
bool logMessageActive = false;
// When was the message shown?
unsigned long messageDisplayMillis = 0;

// Sensor data
float sensorTemp = 0.0;
int sensorHumi = 0;
int sensorPressure = 0;
int sensorMSLP = 0;

float sensorAmbTemp = 0.0;
int sensorAmbHumi;

bool countdownActive = false;
unsigned long countdownDurationSec = 0;
unsigned long countdownTargetMillis = 0;
long countdownPausedRemainingMs = 0;

uint16_t hexToRGB565(String hex) {
  if (hex.startsWith("#")) {
    hex = hex.substring(1);
  }
  if (hex.length() != 6)
    return 0xFFFF;

  long number = strtol(hex.c_str(), NULL, 16);
  uint8_t r = (number >> 16) & 0xFF;
  uint8_t g = (number >> 8) & 0xFF;
  uint8_t b = number & 0xFF;

  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
