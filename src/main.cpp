// SPDX-License-Identifier: GPL-3.0-or-later

/*
ESP32 Matrix Clock - Copyright (C) 2021 Bogdan Sass

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <ArduinoOTA.h>
#include <Ticker.h>
#include <esp_task_wdt.h>

#include "clock.h"
#include "common.h"
#include "creds_mqtt.h"
#include "env_sensor.h"
#include "main.h"
#include "mqtt.h"
#include "rgb_display.h"
#include "settings.h"
#include "web_config.h"
#include <Fonts/TomThumb.h>

Ticker displayTicker;
unsigned long prevEpoch;
unsigned long lastNTPUpdate;
unsigned long lastAmbUpdate;

// Just a blinking heart to show the main thread is still alive...
bool blinkOn;
uint8_t rainbowWheelval = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  display_init();


  displayTest(1000);

  loadSettings();

  env_sensor_init();

  logStatusMessage("Connecting to WiFi!");
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    logStatusMessage("Setup Mode (Connect to MorphClock)");
    initWebConfigAP();
    Serial.println("Started Web Config AP");

    dma_display->clearScreen();
    dma_display->setTextSize(1);
    dma_display->setTextColor(0xFFFF);
    dma_display->setFont(&TomThumb);
    dma_display->setCursor(2, 10);
    dma_display->print("AP: MorphClock");
    dma_display->setCursor(2, 20);
    dma_display->print("192.168.4.1");

    unsigned long waitStart = millis();
    while (millis() - waitStart < 10000) {
      handleWebConfig();
      delay(10);
    }

    dma_display->clearScreen();
    displayTicker.attach_ms(30, displayUpdater);
    // Don't proceed to NTP, MQTT etc.
    return;
  }

  Serial.println("WiFi connected.");
  logStatusMessage("WiFi connected!");
  // Start web config on local network too
  initWebServer();

  dma_display->clearScreen();
  dma_display->setTextSize(1);
  dma_display->setTextColor(0xFFFF);
  dma_display->setFont(&TomThumb);
  dma_display->setCursor(2, 10);
  dma_display->print(WiFi.getHostname());
  dma_display->setCursor(2, 20);
  dma_display->print(WiFi.localIP().toString());

  unsigned long waitStart = millis();
  while (millis() - waitStart < 10000) {
    handleWebServer();
    delay(10);
  }

  logStatusMessage("Setup OTA...");
  ArduinoOTA.onStart([]() {
    switchLayout(0);
    logStatusMessage("OTA Start...");
  });
  ArduinoOTA.onEnd([]() { logStatusMessage("OTA End!"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char buf[32];
    sprintf(buf, "OTA: %u%%", (progress / (total / 100)));
    logStatusMessage(buf);
  });
  ArduinoOTA.onError([](ota_error_t error) { logStatusMessage("OTA Error!"); });
  ArduinoOTA.setHostname(hostname.c_str());
  ArduinoOTA.begin();

  logStatusMessage("NTP time...");
  configTzTime(timezone_str.c_str(), ntp_server.c_str());
  lastNTPUpdate = millis();
  logStatusMessage("NTP done!");

  logStatusMessage("MQTT connect...");

  client.setServer(mqtt_server.c_str(), mqtt_port);
  client.setCallback(mqtt_callback);
  reconnect();
  lastStatusSend = 0;
  logStatusMessage("MQTT done!");

  logStatusMessage("Setting up watchdog...");

  esp_task_wdt_config_t wdt_config = {
      .timeout_ms =
          // New API expects milliseconds, not seconds
      WDT_TIMEOUT * 1000,
      // Watch both CPU cores (0 and 1)
      .idle_core_mask = (1 << 0) | (1 << 1),
      // Reboot on timeout
      .trigger_panic = true};
  esp_task_wdt_reconfigure(&wdt_config);
  esp_task_wdt_add(NULL);
  logStatusMessage("Woof!");

  logStatusMessage(WiFi.localIP().toString());

  logStatusMessage("Getting T/h...");
  getAmbientalData();
  logStatusMessage("Amb recvd!");
  lastAmbUpdate = millis();
  displaySensorData();

  currentLayout = default_layout;
  applyLayout(currentLayout);

  Serial.println("Starting Ticker...");
  dma_display->clearScreen();
  // 30ms for 33 FPS - needed to keep animations smooth
  displayTicker.attach_ms(30, displayUpdater);
}
void loop() {
  if (setupModeActive) {
    handleWebConfig();
    esp_task_wdt_reset();
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    logStatusMessage("WiFi lost!");
    WiFi.reconnect();
  }

  // Handle normal web server requests
  handleWebServer();

  if (!client.connected()) {
    logStatusMessage("MQTT lost");
    reconnect();
  }
  client.loop();

  // Periodically refresh NTP time
  if (millis() - lastNTPUpdate > 1000 * ntp_refresh_interval) {
    logStatusMessage("NTP Refresh");
    configTzTime(timezone_str.c_str(), ntp_server.c_str());
    lastNTPUpdate = millis();
  }

  // Ambient data fetch every amb_refresh_interval seconds
  if (millis() - lastAmbUpdate > 1000 * amb_refresh_interval) {
    getAmbientalData();
    lastAmbUpdate = millis();
  }

  // Touch button handling for layout toggle
  static bool buttonPressed = false;
  static unsigned long lastButtonPress = 0;

  int touchVal = touchRead(pin_touch_button);

  if (touchVal > 0 && touchVal < touch_threshold) {
    if (!buttonPressed && (millis() - lastButtonPress > 500)) {
      buttonPressed = true;
      lastButtonPress = millis();

      int newLayout = (currentLayout + 1) % 3;
      switchLayout(newLayout);
    }
  } else {
    buttonPressed = false;
  }

  ArduinoOTA.handle();

  // Do we need to clear the status message from the screen?
  if (logMessageActive) {
    if (millis() > messageDisplayMillis + LOG_MESSAGE_PERSISTENCE_MSEC) {
      clearStatusMessage();
    }
  }

  // Is the sensor data too old?
  if (millis() - lastSensorRead > 1000 * sensor_dead_interval) {
    sensorDead = true;
  }

  // 10s display toggle logic
  static unsigned long lastToggleTime = 0;
  static bool showAmbient = true;
  static bool forceRedraw = true;

  if (millis() - lastToggleTime > 10000) {
    showAmbient = !showAmbient;
    lastToggleTime = millis();
    forceRedraw = true;
  }

  if (newSensorData && !showAmbient) {
    forceRedraw = true;
  }

  if (forceRedraw) {
    if (currentLayout == 1) {
      displayAmbientalData();
      displaySensorData();
    } else if (currentLayout == 0) {
      if (showAmbient) {
        displayAmbientalData();
      } else {
        displaySensorData();
      }
    }
    forceRedraw = false;
  }

  // Scrolling info (IP and Hostname) at the bottom
  static unsigned long lastScrollTime = 0;
  if (millis() - lastScrollTime > 50) {
    drawScrollingInfo();
    lastScrollTime = millis();
  }

  // Reset the watchdog timer as long as the main task is running
  esp_task_wdt_reset();

  delay(500);
}

void displayUpdater() {
  if (setupModeActive) {
    static uint8_t tick_count = 0;
    if (++tick_count >= 5) {
      rainbowWheelval++;
      tick_count = 0;
    }
    drawRainbowBorder(rainbowWheelval);
    return;
  }

  if (!getLocalTime(&timeinfo)) {
    logStatusMessage("Failed to get time!");
    return;
  }

  uint8_t new_brightness = display_brightness;
  // 10 PM to 11:59 PM
  if (timeinfo.tm_hour >= 22) {
    new_brightness = display_brightness / 2;
    // 12 AM to 7 AM
  } else if (timeinfo.tm_hour >= 0 && timeinfo.tm_hour < 7) {
    new_brightness = display_brightness / 4;
  } else {
    new_brightness = display_brightness;
  }

  static uint8_t current_brightness = 0;
  if (current_brightness != new_brightness) {
    dma_display->setBrightness8(new_brightness);
    current_brightness = new_brightness;
  }

  unsigned long epoch = mktime(&timeinfo);
  if (epoch != prevEpoch) {
    if (currentLayout != 2) {
      displayClock();
    }
    prevEpoch = epoch;
  }

  if (currentLayout == 2) {
    displayCountdown();
  }

  static uint8_t tick_count = 0;

  // 30ms * 5 = 150ms per step (slow & smooth fading)
  if (++tick_count >= 5) {
    rainbowWheelval++;
    tick_count = 0;
  }
  drawRainbowBorder(rainbowWheelval);

  // Smoothly fade heartbeat in the Ticker
  drawHeartBeat();
}

// TODO: http://www.rinkydinkelectronics.com/t_imageconverter565.php

// TODO - add heartbeat in loop(), reboot in interrupt if heartbeat lost
// (sometimes the system hangs during OTA request)
// https://iotassistant.io/esp32/enable-hardware-watchdog-timer-esp32-arduino-ide/

// TODO - get and print weather forecast every X interval (4h?)
// TODO - use light sensor data to set display brightness
// TODO - add option to turn off display via MQTT
// TODO - replace bitmap arrays with color565 values!
// TODO - add event-based wifi disconnect/reconnect -
// https://randomnerdtutorials.com/solved-reconnect-esp32-to-wifi/

// TODO - check asynchronously for buzzer stop
// TODO - move TSL read to async task

void switchLayout(int newLayout) {
  if (currentLayout == newLayout)
    return;
  applyLayout(newLayout);
  clockStartingUp = true;
  dma_display->clearScreen();
  drawRainbowBorder(rainbowWheelval);
  logStatusMessage(String("Switched to Layout ") + newLayout);
}
