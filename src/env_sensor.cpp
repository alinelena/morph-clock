#include "env_sensor.h"
#include "common.h"
#include "config.h"
#include "settings.h"
#include <Wire.h>

#include <Adafruit_BME280.h>
#include <Adafruit_AHTX0.h>

Adafruit_BME280 bme;
Adafruit_AHTX0 aht;

void env_sensor_init() {
  if (active_sensor == SENSOR_TYPE_BME280) {
    Wire.begin(BME280_SDA_PIN, BME280_SCL_PIN);
    if (!bme.begin(0x76)) {
      Serial.println("BME280 Sensor NOT found! Check wiring.");
    }
  } else if (active_sensor == SENSOR_TYPE_AHT20) {
    Wire.begin(AHT20_SDA_PIN, AHT20_SCL_PIN);
    if (!aht.begin()) {
      Serial.println("AHT20 Sensor NOT found! Check wiring.");
    }
  }
}

void getAmbientalData() {
  if (active_sensor == SENSOR_TYPE_BME280) {
    sensorAmbTemp = bme.readTemperature();
    sensorAmbHumi = int(bme.readHumidity());
    float absPressure = bme.readPressure() / 100.0F; // Convert Pa to hPa
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
