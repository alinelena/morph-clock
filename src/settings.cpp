#include "settings.h"
#include "creds_mqtt.h" // Fallbacks

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
int active_sensor = 2; // Default to BME280
int display_brightness = 128;
String timezone_str = "GMT0BST,M3.5.0/1,M10.5.0";
int altitude_meters = 35;

void loadSettings() {
  Preferences preferences;
  preferences.begin("morphclock", true); // Read-only

  wifi_ssid = preferences.getString("wifi_ssid", WIFI_SSID);
  wifi_password = preferences.getString("wifi_pass", WIFI_PASSWORD);

  mqtt_server = preferences.getString("mqtt_server", MQTT_SERVER);
  mqtt_port = preferences.getInt("mqtt_port", MQTT_PORT);
  mqtt_username = preferences.getString("mqtt_user", MQTT_USERNAME);
  mqtt_password = preferences.getString("mqtt_pass", MQTT_PASSWORD);

  mqtt_temp_topic = preferences.getString("mqtt_temp", MQTT_TEMPERATURE_SENSOR_TOPIC);
  mqtt_humi_topic = preferences.getString("mqtt_humi", MQTT_HUMIDITY_SENSOR_TOPIC);
  mqtt_pres_topic = preferences.getString("mqtt_pres", MQTT_PRESSURE_SENSOR_TOPIC);
  mqtt_layout_topic = preferences.getString("mqtt_layout", MQTT_LAYOUT_TOPIC);
  mqtt_count_topic = preferences.getString("mqtt_count", MQTT_COUNTDOWN_TOPIC);

  l1_amb_color = preferences.getString("l1_amb_col", "#00FFFF");
  l1_out_color = preferences.getString("l1_out_col", "#FF00FF"); // Default pink
  l1_clock_color = preferences.getString("l1_clk_col", "#0000FF"); // Default blue
  l1_date_color = preferences.getString("l1_dat_col", "#00FF00"); // Default green
  l1_line_color = preferences.getString("l1_lin_col", "#FFFF00"); // Default yellow
  l2_amb_color = preferences.getString("l2_amb_col", "#DC143C"); // Crimson
  l2_out_color = preferences.getString("l2_out_col", "#FF00FF"); // Pink
  l2_clock_color = preferences.getString("l2_color", "#00FFFF");
  l2_date_color = preferences.getString("l2_dat_col", "#00FF00");
  log_msg_color = preferences.getString("log_color", "#FFFF00"); // Yellow
  scroll_msg_color = preferences.getString("scroll_col", "#FFFF00");
  l3_color_start = preferences.getString("l3_start", "#00FF00");
  l3_color_warn = preferences.getString("l3_warn", "#FFFF00");
  l3_color_end = preferences.getString("l3_end", "#FF0000");
  l3_yellow_threshold = preferences.getInt("l3_thresh", 20);
  l3_default_countdown = preferences.getInt("l3_def_cnt", 60);
  default_layout = preferences.getInt("def_layout", 1);
  active_sensor = preferences.getInt("act_sensor", 2);
  display_brightness = preferences.getInt("brightness", 128); // Default 128
  timezone_str = preferences.getString("tz", "GMT0BST,M3.5.0/1,M10.5.0"); // Default GMT/BST
  altitude_meters = preferences.getInt("altitude", 35);

  preferences.end();
}

void saveSettings() {
  Preferences preferences;
  preferences.begin("morphclock", false); // Read-write

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
  preferences.putInt("altitude", altitude_meters);

  preferences.end();
}
