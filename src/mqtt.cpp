
#include <Arduino.h>
// #include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "common.h"
#include "config.h"
#include "creds_mqtt.h"
#include "main.h"
#include "mqtt.h"
#include "settings.h"

char mqtt_buffer[MQTT_BUFMAX];

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  // handle message arrived
  /*
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  */

  if (strcmp(topic, mqtt_temp_topic.c_str()) == 0) {
    payload[length] = 0;
    sensorTemp = atof((char *)payload);
    lastSensorRead = millis();
    sensorDead = false;
    newSensorData = true;
  }

  if (strcmp(topic, mqtt_humi_topic.c_str()) == 0) {
    payload[length] = 0;
    sensorHumi = atoi((char *)payload);
    lastSensorRead = millis();
    sensorDead = false;
    newSensorData = true;
  }

#if ACTIVE_SENSOR != SENSOR_TYPE_BME280
  if (strcmp(topic, mqtt_pres_topic.c_str()) == 0) {
    payload[length] = 0;
    sensorPressure = atoi((char *)payload);
    sensorMSLP = sensorPressure;
    lastSensorRead = millis();
    sensorDead = false;
    newSensorData = true;
  }
#endif

  if (strcmp(topic, mqtt_count_topic.c_str()) == 0) {
    payload[length] = 0;
    countdownDurationSec = atol((char *)payload);
    countdownTargetMillis = millis() + (countdownDurationSec * 1000);
    countdownActive = true;
    switchLayout(2);
  }

  if (strcmp(topic, mqtt_layout_topic.c_str()) == 0) {
    payload[length] = 0;
    if (strcmp((char *)payload, "1") == 0) {
      // L1
      switchLayout(1);
    } else if (strcmp((char *)payload, "2") == 0) {
      // L2 (which corresponds to Layout 0 internally)
      switchLayout(0);
    } else if (strcmp((char *)payload, "3") == 0) {
      // Layout 2 (Countdown)
      switchLayout(2);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if (status != WL_CONNECTED) {
      WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to MQTT node...");

    client.setServer(mqtt_server.c_str(), mqtt_port);
    // Attempt to connect (clientId, username, password)
    if (client.connect(HOSTNAME, mqtt_username.c_str(),
                       mqtt_password.c_str())) {
      Serial.println("[DONE]");
      Serial.println("Subscribing to topics:");
      Serial.println(mqtt_temp_topic);
      Serial.println(mqtt_humi_topic);
      Serial.println(mqtt_pres_topic);
      Serial.println("... done");

      client.subscribe(mqtt_temp_topic.c_str());
      client.subscribe(mqtt_humi_topic.c_str());
#if ACTIVE_SENSOR != SENSOR_TYPE_BME280
      client.subscribe(mqtt_pres_topic.c_str());
#endif
      client.subscribe(mqtt_layout_topic.c_str());
      client.subscribe(mqtt_count_topic.c_str());
    } else {
      logStatusMessage("MQTT Fail, retrying...");
      Serial.print("[FAILED] [ rc = ");
      Serial.print(client.state());
      Serial.println(" : retrying in 5 seconds]");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
