// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

extern String wifi_ssid;
extern String wifi_password;

extern String mqtt_server;
extern int mqtt_port;
extern String mqtt_username;
extern String mqtt_password;

extern String mqtt_temp_topic;
extern String mqtt_humi_topic;
extern String mqtt_pres_topic;
extern String mqtt_layout_topic;
extern String mqtt_count_topic;

extern String l1_amb_color;
extern String l1_out_color;
extern String l1_clock_color;
extern String l1_date_color;
extern String l1_line_color;
extern String l2_amb_color;
extern String l2_out_color;
extern String l2_clock_color;
extern String l2_date_color;
extern String log_msg_color;
extern String scroll_msg_color;
extern String l3_color_start;
extern String l3_color_warn;
extern String l3_color_end;
extern int l3_yellow_threshold;
extern int l3_default_countdown;
extern int default_layout;
extern int active_sensor;
extern int display_brightness;
extern String timezone_str;
extern String ntp_server;
extern int ntp_refresh_interval;
extern String hostname;
extern int altitude_meters;
extern int amb_refresh_interval;
extern int sensor_dead_interval;

extern int8_t pin_r1;
extern int8_t pin_g1;
extern int8_t pin_b1;
extern int8_t pin_r2;
extern int8_t pin_g2;
extern int8_t pin_b2;
extern int8_t pin_a;
extern int8_t pin_b;
extern int8_t pin_c;
extern int8_t pin_d;
extern int8_t pin_e;
extern int8_t pin_ldr;
extern int8_t pin_irq;
extern int8_t pin_lat;
extern int8_t pin_oe;
extern int8_t pin_clk;

extern int8_t pin_touch_button;
extern int touch_threshold;
extern int8_t pin_aht20_sda;
extern int8_t pin_aht20_scl;
extern int8_t pin_bme280_sda;
extern int8_t pin_bme280_scl;

void loadSettings();
void saveSettings();

#endif
