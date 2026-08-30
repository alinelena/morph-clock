// SPDX-License-Identifier: GPL-3.0-or-later

#include "env_sensor.h"
#include "common.h"
#include "config.h"
#include "settings.h"
#include <Wire.h>

#include <Adafruit_AHTX0.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
Adafruit_AHTX0 aht;

void env_sensor_init() {
  if (active_sensor == SENSOR_TYPE_BME280) {
    Wire.begin(pin_bme280_sda, pin_bme280_scl);
    if (!bme.begin(0x76)) {
      Serial.println("BME280 Sensor NOT found! Check wiring.");
    }
  } else if (active_sensor == SENSOR_TYPE_AHT20) {
    Wire.begin(pin_aht20_sda, pin_aht20_scl);
    if (!aht.begin()) {
      Serial.println("AHT20 Sensor NOT found! Check wiring.");
    }
  }
}

void getAmbientalData() {
  if (active_sensor == SENSOR_TYPE_BME280) {
    sensorAmbTemp = bme.readTemperature();
    sensorAmbHumi = int(bme.readHumidity());
    // Convert Pa to hPa
    float absPressure = bme.readPressure() / 100.0F;
    sensorPressure = int(absPressure);
    sensorMSLP = int(bme.seaLevelForAltitude(altitude_meters, absPressure));
  } else if (active_sensor == SENSOR_TYPE_AHT20) {
    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
      sensorAmbTemp = temp.temperature;
      sensorAmbHumi = int(humidity.relative_humidity);
    } else {
      Serial.println("Failed to read AHT20 sensor!");
    }
  }
}
