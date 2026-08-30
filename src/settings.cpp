// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"
// Fallbacks
#include "creds_mqtt.h"
#include "rgb_display.h"

String wifi_ssid;
String wifi_password;

String mqtt_server;
int mqtt_port;
String mqtt_username;
String mqtt_password;

String mqtt_temp_topic;
String mqtt_humi_topic;
String mqtt_pres_topic;
String mqtt_layout_topic;
String mqtt_count_topic;

String l1_amb_color;
String l1_out_color;
String l1_clock_color;
String l1_date_color;
String l1_line_color;
String l2_amb_color;
String l2_out_color;
String l2_clock_color;
String l2_date_color;
String log_msg_color;
String scroll_msg_color;
String l3_color_start;
String l3_color_warn;
String l3_color_end;
int l3_yellow_threshold = 20;
int l3_default_countdown = 60;
int default_layout = 1;
// Default to BME280
int active_sensor = 2;
int display_brightness = 128;
String timezone_str = "GMT0BST,M3.5.0/1,M10.5.0";
int altitude_meters = 35;
String ntp_server = NTP_SERVER;
int ntp_refresh_interval = NTP_REFRESH_INTERVAL_SEC;
String hostname = HOSTNAME;
int amb_refresh_interval = AMB_REFRESH_INTERVAL_SEC;
int sensor_dead_interval = SENSOR_DEAD_INTERVAL_SEC;

int pin_r1 = R1_PIN;
int pin_g1 = G1_PIN;
int pin_b1 = B1_PIN;
int pin_r2 = R2_PIN;
int pin_g2 = G2_PIN;
int pin_b2 = B2_PIN;
int pin_a = A_PIN;
int pin_b = B_PIN;
int pin_c = C_PIN;
int pin_d = D_PIN;
int pin_e = E_PIN;
int pin_ldr = LDR_PIN;
int pin_irq = IRQ_PIN;
int pin_lat = LAT_PIN;
int pin_oe = OE_PIN;
int pin_clk = CLK_PIN;

int pin_touch_button = TOUCH_BUTTON_PIN;

int pin_aht20_sda = AHT20_SDA_PIN;
int pin_aht20_scl = AHT20_SCL_PIN;
int pin_bme280_sda = BME280_SDA_PIN;
int pin_bme280_scl = BME280_SCL_PIN;

void loadSettings() {
  Preferences preferences;
  // Read-only
  preferences.begin("morphclock", true);

  wifi_ssid = preferences.getString("wifi_ssid", WIFI_SSID);
  wifi_password = preferences.getString("wifi_pass", WIFI_PASSWORD);

  mqtt_server = preferences.getString("mqtt_server", MQTT_SERVER);
  mqtt_port = preferences.getInt("mqtt_port", MQTT_PORT);
  mqtt_username = preferences.getString("mqtt_user", MQTT_USERNAME);
  mqtt_password = preferences.getString("mqtt_pass", MQTT_PASSWORD);

  mqtt_temp_topic =
      preferences.getString("mqtt_temp", MQTT_TEMPERATURE_SENSOR_TOPIC);
  mqtt_humi_topic =
      preferences.getString("mqtt_humi", MQTT_HUMIDITY_SENSOR_TOPIC);
  mqtt_pres_topic =
      preferences.getString("mqtt_pres", MQTT_PRESSURE_SENSOR_TOPIC);
  mqtt_layout_topic = preferences.getString("mqtt_layout", MQTT_LAYOUT_TOPIC);
  mqtt_count_topic = preferences.getString("mqtt_count", MQTT_COUNTDOWN_TOPIC);

  l1_amb_color = preferences.getString("l1_amb_col", "#00FFFF");
  // Default pink
  l1_out_color = preferences.getString("l1_out_col", "#FF00FF");
  // Default blue
  l1_clock_color = preferences.getString("l1_clk_col", "#0000FF");
  // Default green
  l1_date_color = preferences.getString("l1_dat_col", "#00FF00");
  // Default yellow
  l1_line_color = preferences.getString("l1_lin_col", "#FFFF00");
  // Crimson
  l2_amb_color = preferences.getString("l2_amb_col", "#DC143C");
  // Pink
  l2_out_color = preferences.getString("l2_out_col", "#FF00FF");
  l2_clock_color = preferences.getString("l2_color", "#00FFFF");
  l2_date_color = preferences.getString("l2_dat_col", "#00FF00");
  // Yellow
  log_msg_color = preferences.getString("log_color", "#FFFF00");
  scroll_msg_color = preferences.getString("scroll_col", "#FFFF00");
  l3_color_start = preferences.getString("l3_start", "#00FF00");
  l3_color_warn = preferences.getString("l3_warn", "#FFFF00");
  l3_color_end = preferences.getString("l3_end", "#FF0000");
  l3_yellow_threshold = preferences.getInt("l3_thresh", 20);
  l3_default_countdown = preferences.getInt("l3_def_cnt", 60);
  default_layout = preferences.getInt("def_layout", 1);
  active_sensor = preferences.getInt("act_sensor", 2);
  // Default 128
  display_brightness = preferences.getInt("brightness", 128);
  // Default GMT/BST
  timezone_str = preferences.getString("tz", "GMT0BST,M3.5.0/1,M10.5.0");
  ntp_server = preferences.getString("ntp_srv", NTP_SERVER);
  ntp_refresh_interval = preferences.getInt("ntp_int", NTP_REFRESH_INTERVAL_SEC);
  hostname = preferences.getString("hostname", HOSTNAME);
  amb_refresh_interval = preferences.getInt("amb_int", AMB_REFRESH_INTERVAL_SEC);
  sensor_dead_interval = preferences.getInt("dead_int", SENSOR_DEAD_INTERVAL_SEC);
  altitude_meters = preferences.getInt("altitude", 35);

  pin_r1 = preferences.getInt("pin_r1", R1_PIN);
  pin_g1 = preferences.getInt("pin_g1", G1_PIN);
  pin_b1 = preferences.getInt("pin_b1", B1_PIN);
  pin_r2 = preferences.getInt("pin_r2", R2_PIN);
  pin_g2 = preferences.getInt("pin_g2", G2_PIN);
  pin_b2 = preferences.getInt("pin_b2", B2_PIN);
  pin_a = preferences.getInt("pin_a", A_PIN);
  pin_b = preferences.getInt("pin_b", B_PIN);
  pin_c = preferences.getInt("pin_c", C_PIN);
  pin_d = preferences.getInt("pin_d", D_PIN);
  pin_e = preferences.getInt("pin_e", E_PIN);
  pin_ldr = preferences.getInt("pin_ldr", LDR_PIN);
  pin_irq = preferences.getInt("pin_irq", IRQ_PIN);
  pin_lat = preferences.getInt("pin_lat", LAT_PIN);
  pin_oe = preferences.getInt("pin_oe", OE_PIN);
  pin_clk = preferences.getInt("pin_clk", CLK_PIN);

  pin_touch_button = preferences.getInt("pin_touch", TOUCH_BUTTON_PIN);

  pin_aht20_sda = preferences.getInt("pin_aht_sda", AHT20_SDA_PIN);
  pin_aht20_scl = preferences.getInt("pin_aht_scl", AHT20_SCL_PIN);
  pin_bme280_sda = preferences.getInt("pin_bme_sda", BME280_SDA_PIN);
  pin_bme280_scl = preferences.getInt("pin_bme_scl", BME280_SCL_PIN);

  preferences.end();
}

void saveSettings() {
  Preferences preferences;
  // Read-write
  preferences.begin("morphclock", false);

  preferences.putString("wifi_ssid", wifi_ssid);
  preferences.putString("wifi_pass", wifi_password);

  preferences.putString("mqtt_server", mqtt_server);
  preferences.putInt("mqtt_port", mqtt_port);
  preferences.putString("mqtt_user", mqtt_username);
  preferences.putString("mqtt_pass", mqtt_password);

  preferences.putString("mqtt_temp", mqtt_temp_topic);
  preferences.putString("mqtt_humi", mqtt_humi_topic);
  preferences.putString("mqtt_pres", mqtt_pres_topic);
  preferences.putString("mqtt_layout", mqtt_layout_topic);
  preferences.putString("mqtt_count", mqtt_count_topic);

  preferences.putString("l1_amb_col", l1_amb_color);
  preferences.putString("l1_out_col", l1_out_color);
  preferences.putString("l1_clk_col", l1_clock_color);
  preferences.putString("l1_dat_col", l1_date_color);
  preferences.putString("l1_lin_col", l1_line_color);
  preferences.putString("l2_amb_col", l2_amb_color);
  preferences.putString("l2_out_col", l2_out_color);
  preferences.putString("l2_color", l2_clock_color);
  preferences.putString("l2_dat_col", l2_date_color);
  preferences.putString("log_color", log_msg_color);
  preferences.putString("scroll_col", scroll_msg_color);
  preferences.putString("l3_start", l3_color_start);
  preferences.putString("l3_warn", l3_color_warn);
  preferences.putString("l3_end", l3_color_end);
  preferences.putInt("l3_thresh", l3_yellow_threshold);
  preferences.putInt("l3_def_cnt", l3_default_countdown);
  preferences.putInt("def_layout", default_layout);
  preferences.putInt("act_sensor", active_sensor);
  preferences.putInt("brightness", display_brightness);
  preferences.putString("tz", timezone_str);
  preferences.putString("ntp_srv", ntp_server);
  preferences.putInt("ntp_int", ntp_refresh_interval);
  preferences.putString("hostname", hostname);
  preferences.putInt("amb_int", amb_refresh_interval);
  preferences.putInt("dead_int", sensor_dead_interval);
  preferences.putInt("altitude", altitude_meters);

  preferences.putInt("pin_r1", pin_r1);
  preferences.putInt("pin_g1", pin_g1);
  preferences.putInt("pin_b1", pin_b1);
  preferences.putInt("pin_r2", pin_r2);
  preferences.putInt("pin_g2", pin_g2);
  preferences.putInt("pin_b2", pin_b2);
  preferences.putInt("pin_a", pin_a);
  preferences.putInt("pin_b", pin_b);
  preferences.putInt("pin_c", pin_c);
  preferences.putInt("pin_d", pin_d);
  preferences.putInt("pin_e", pin_e);
  preferences.putInt("pin_ldr", pin_ldr);
  preferences.putInt("pin_irq", pin_irq);
  preferences.putInt("pin_lat", pin_lat);
  preferences.putInt("pin_oe", pin_oe);
  preferences.putInt("pin_clk", pin_clk);

  preferences.putInt("pin_touch", pin_touch_button);

  preferences.putInt("pin_aht_sda", pin_aht20_sda);
  preferences.putInt("pin_aht_scl", pin_aht20_scl);
  preferences.putInt("pin_bme_sda", pin_bme280_sda);
  preferences.putInt("pin_bme_scl", pin_bme280_scl);

  preferences.end();
}
