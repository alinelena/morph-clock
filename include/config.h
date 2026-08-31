// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "1.1.0"

// --- Hardware Settings ---
#define SENSOR_TYPE_AHT20 1
#define SENSOR_TYPE_BME280 2

// Sensor Altitude (in meters) for Mean Sea Level Pressure (MSLP) correction
// Warrington UK is ~32m above sea level. First floor adds ~3m.
// Set this to 32 if you move the clock to the ground floor.

// I2C Pins (BME280 has SDA/SCL swapped compared to AHT20)
#define AHT20_SDA_PIN 22
#define AHT20_SCL_PIN 21

#define BME280_SDA_PIN 21
#define BME280_SCL_PIN 22

// How often we refresh the time from the NTP server
#define NTP_REFRESH_INTERVAL_SEC 3600
#define NTP_SERVER "time.google.com"

// How long are informational messages kept on screen
#define LOG_MESSAGE_PERSISTENCE_MSEC 30000

// How long do we consider the sensor data valid before declaring the sensor
// dead
#define SENSOR_DEAD_INTERVAL_SEC 600

// Button pin
#define TOUCH_BUTTON_PIN 32
#define TOUCH_THRESHOLD 400

// Photoresistor (LDR) pin
#define LDR_PIN 35

// I2C Interrupt (IRQ) pin
#define IRQ_PIN 34

// Hostname for the device
#define HOSTNAME "rubicante"

// Screen positioning settings
// Panel size
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32

// Layout 0: Small clock, small date/DOW in bottom right, heartbeat top right
#define L2_CLOCK_X 1
#define L2_CLOCK_Y 1
#define L2_CLOCK_SEGMENT_HEIGHT 6
#define L2_CLOCK_SEGMENT_WIDTH 3
#define L2_CLOCK_SEGMENT_SPACING 3
#define L2_DOW_X 45
#define L2_DOW_Y 15
#define L2_DATE_X 42
#define L2_DATE_Y 22
#define L2_DATE_WIDTH 21
#define L2_DATE_HEIGHT 14
#define L2_HEARTBEAT_X 55
#define L2_HEARTBEAT_Y 1

// Layout 1: Large clock on bottom half, date/DOW on right, sensor data on top
#define L1_CLOCK_X 1
#define L1_CLOCK_Y 16
#define L1_CLOCK_SEGMENT_HEIGHT 6
#define L1_CLOCK_SEGMENT_WIDTH 3
#define L1_CLOCK_SEGMENT_SPACING 3
#define L1_DOW_X 48
#define L1_DOW_Y 22
#define L1_DATE_X 44
#define L1_DATE_Y 30
#define L1_DATE_WIDTH 19
#define L1_DATE_HEIGHT 15
#define L1_HEARTBEAT_X 55
#define L1_HEARTBEAT_Y 1
#define L1_PART1_X 1
#define L1_PART2_X 22
#define L1_PART3_X 43
#define L1_PART_WIDTH 20
#define L1_TEMP_Y 7
#define L1_HUMI_Y 14

// Layout 2: Countdown Timer
#define COUNTDOWN_CLOCK_X 11
#define COUNTDOWN_CLOCK_Y 4
#define COUNTDOWN_CLOCK_SEGMENT_HEIGHT 10
#define COUNTDOWN_CLOCK_SEGMENT_WIDTH 4
#define COUNTDOWN_CLOCK_SEGMENT_SPACING 2

// Sensor data layout config
#define L2_SENSOR_DATA_X 1
#define L2_SENSOR_DATA_Y 16

// Colors and shared settings
#define CLOCK_ANIMATION_DELAY_MSEC 50
// Colors managed via Web UI

// Weather sensor data
#define SENSOR_DATA_WIDTH 40
#define SENSOR_DATA_HEIGHT 8
#define SENSOR_ERROR_DATA_COLOR                                                \
  ((0xFF & 0xF8) << 8) | ((0x00 & 0xFC) << 3) | (0x00 >> 3)

// Ambiental sensor data
#define SENSOR_AMB_DATA_WIDTH 40
#define SENSOR_AMB_DATA_HEIGHT 8
#define AMB_REFRESH_INTERVAL_SEC 30

// Watchdog settings
#define WDT_TIMEOUT 60

#endif
