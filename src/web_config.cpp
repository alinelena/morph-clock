// SPDX-License-Identifier: GPL-3.0-or-later

#include "web_config.h"
#include "config.h"
#include "settings.h"
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer webServer(80);

bool setupModeActive = false;

const char *html_page PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Morphing Clock Settings</title>
    <style>
        :root {
            --bg-grad-1: #0f172a;
            --bg-grad-2: #334155;
            --glass-bg: rgba(255, 255, 255, 0.05);
            --glass-border: rgba(255, 255, 255, 0.1);
            --primary: #3b82f6;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Inter', sans-serif;
            background: linear-gradient(135deg, var(--bg-grad-1), var(--bg-grad-2));
            color: var(--text-main);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            background: var(--glass-bg);
            backdrop-filter: blur(12px);
            border: 1px solid var(--glass-border);
            border-radius: 24px;
            padding: 30px;
            width: 100%;
            max-width: 600px;
            margin: 0 auto;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
        }
        h1 { text-align: center; margin-bottom: 20px; background: linear-gradient(to right, #60a5fa, #a78bfa); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
        
        .tabs { display: flex; flex-wrap: wrap; gap: 5px; border-bottom: 1px solid var(--glass-border); margin-bottom: 20px; }
        .tab { padding: 12px 20px; cursor: pointer; color: var(--text-muted); font-weight: 600; border-bottom: 2px solid transparent; transition: all 0.3s; }
        .tab.active { color: var(--primary); border-bottom-color: var(--primary); }
        .tab-content { display: none; animation: fadeIn 0.4s ease; }
        .tab-content.active { display: block; }
        @keyframes fadeIn { from { opacity: 0; transform: translateY(10px); } to { opacity: 1; transform: translateY(0); } }
        
        .section-title { font-size: 15px; font-weight: 600; color: var(--text-muted); margin: 25px 0 15px; border-bottom: 1px solid var(--glass-border); padding-bottom: 8px; text-transform: uppercase; }
        .form-group { margin-bottom: 15px; }
        label { display: block; font-size: 13px; color: var(--text-muted); margin-bottom: 6px; font-weight: 500; }
        input[type="text"], input[type="password"], input[type="number"], select, input[type="color"] {
            width: 100%; background: rgba(0, 0, 0, 0.2); border: 1px solid var(--glass-border); border-radius: 12px; padding: 12px; color: #fff; font-size: 15px;
        }
        input[type="color"] { padding: 5px; height: 45px; cursor: pointer; }
        button {
            width: 100%; background: linear-gradient(135deg, var(--primary), #8b5cf6); color: white; border: none; border-radius: 12px; padding: 15px; font-size: 16px; font-weight: 600; cursor: pointer; margin-top: 20px;
        }
        .btn-group { display: flex; gap: 10px; margin-top: 10px; }
        .btn-group button { margin-top: 0; padding: 12px; font-size: 14px; }
        .btn-stop { background: linear-gradient(135deg, #ef4444, #b91c1c); }
        .btn-reset { background: linear-gradient(135deg, #64748b, #475569); }
        .flex-row { display: flex; gap: 10px; }
        @media (max-width: 600px) {
            body { padding: 10px; }
            .container { padding: 20px 15px; border-radius: 16px; }
            .tabs { flex-wrap: wrap; justify-content: center; border-bottom: none; gap: 5px; }
            .tab { flex: 1 1 45%; text-align: center; padding: 10px 5px; font-size: 13px; border-bottom: 2px solid transparent; background: rgba(0,0,0,0.1); border-radius: 8px; }
            .tab.active { background: rgba(59,130,246,0.1); border-bottom-color: var(--primary); }
            .flex-row { flex-direction: column; gap: 0px; }
            .btn-group { flex-wrap: wrap; }
            .btn-group button { flex: 1 1 45%; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Morphing Clock <span id="fw_version" style="font-size: 14px; color: var(--text-muted); font-weight: normal; vertical-align: middle;"></span></h1>
        
        <div class="tabs">
            <div class="tab active" onclick="switchTab('network')">Network</div>
            <div class="tab" onclick="switchTab('layout')">Layout Settings</div>
            <div class="tab" onclick="switchTab('timer')">Live Timer</div>
            <div class="tab" onclick="switchTab('update')">Firmware Update</div>
            <div class="tab" onclick="switchTab('pins')">Hardware Pins</div>
            <div class="tab" onclick="switchTab('about')">About</div>
        </div>
        
        <form id="configForm" onsubmit="submitForm(event)">
            <div id="network" class="tab-content active">
                <div class="section-title">WiFi Credentials</div>
                <div class="form-group"><label>SSID</label><input type="text" name="wifi_ssid" id="wifi_ssid"></div>
                <div class="form-group"><label>Password</label><input type="password" name="wifi_pass" id="wifi_pass"></div>
                <div class="form-group"><label>Hostname</label><input type="text" name="hostname" id="hostname"></div>

                <div class="section-title">MQTT Settings</div>
                <div class="form-group"><label>Server</label><input type="text" name="mqtt_server" id="mqtt_server"></div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Port</label><input type="number" name="mqtt_port" id="mqtt_port"></div>
                    <div class="form-group" style="flex:1"><label>User</label><input type="text" name="mqtt_user" id="mqtt_user"></div>
                    <div class="form-group" style="flex:1"><label>Pass</label><input type="password" name="mqtt_pass" id="mqtt_pass"></div>
                </div>
                <div class="section-title">MQTT Topics</div>
                <div class="form-group"><label>Temperature</label><input type="text" name="mqtt_temp" id="mqtt_temp"></div>
                <div class="form-group"><label>Humidity</label><input type="text" name="mqtt_humi" id="mqtt_humi"></div>
                <div class="form-group"><label>Pressure</label><input type="text" name="mqtt_pres" id="mqtt_pres"></div>
                <div class="form-group"><label>Layout</label><input type="text" name="mqtt_layout" id="mqtt_layout"></div>
                <div class="form-group"><label>Countdown</label><input type="text" name="mqtt_count" id="mqtt_count"></div>

                <div class="section-title">NTP Settings</div>
                <div class="form-group"><label>NTP Server</label><input type="text" name="ntp_srv" id="ntp_srv"></div>
                <div class="form-group"><label>Refresh Interval (Seconds)</label><input type="number" name="ntp_int" id="ntp_int"></div>
            </div>

            <div id="layout" class="tab-content">
                <div class="section-title">Instant Layout Switch</div>
                <div class="btn-group" style="margin-bottom: 25px;">
                    <button type="button" onclick="apiCall('layout1')" style="background: linear-gradient(135deg, #475569, #334155);">Layer 1</button>
                    <button type="button" onclick="apiCall('layout2')" style="background: linear-gradient(135deg, #475569, #334155);">Layer 2</button>
                    <button type="button" onclick="apiCall('layout3')" style="background: linear-gradient(135deg, #475569, #334155);">Layer 3</button>
                </div>

                <div class="section-title">General Layout Settings</div>
                <div class="form-group">
                    <label>Ambient Sensor Connected</label>
                    <select name="act_sensor" id="act_sensor">
                        <option value="2">BME280 (Temp, Humi, Pres)</option>
                        <option value="1">AHT20 (Temp, Humi)</option>
                        <option value="0">None</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>Default Boot Layout</label>
                    <select name="def_layout" id="def_layout">
                        <option value="1">Layer 1: Time + Environment (Stacked)</option>
                        <option value="0">Layer 2: Time Only (Alternating)</option>
                        <option value="2">Layer 3: Countdown Timer</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>Timezone</label>
                    <select name="tz" id="tz">
                        <option value="GMT0BST,M3.5.0/1,M10.5.0">GMT/BST (UK, Ireland)</option>
                        <option value="CET-1CEST,M3.5.0,M10.5.0/3">CET/CEST (Central Europe)</option>
                        <option value="EET-2EEST,M3.5.0/3,M10.5.0/4">EET/EEST (Eastern Europe)</option>
                        <option value="EST5EDT,M3.2.0,M11.1.0">EST/EDT (US Eastern)</option>
                        <option value="CST6CDT,M3.2.0,M11.1.0">CST/CDT (US Central)</option>
                        <option value="MST7MDT,M3.2.0,M11.1.0">MST/MDT (US Mountain)</option>
                        <option value="PST8PDT,M3.2.0,M11.1.0">PST/PDT (US Pacific)</option>
                        <option value="JST-9">JST (Japan)</option>
                        <option value="AEST-10AEDT,M10.1.0,M4.1.0/3">AEST/AEDT (Sydney, Melbourne)</option>
                        <option value="NZST-12NZDT,M9.5.0,M4.1.0/3">NZST/NZDT (New Zealand)</option>
                    </select>
                </div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Altitude (Meters)</label><input type="number" name="altitude" id="altitude"></div>
                    <div class="form-group" style="flex:1"><label>Brightness (0-255)</label><input type="number" min="0" max="255" name="brightness" id="brightness"></div>
                </div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Sensor Refresh (s)</label><input type="number" name="amb_int" id="amb_int"></div>
                    <div class="form-group" style="flex:1"><label>Sensor Dead (s)</label><input type="number" name="dead_int" id="dead_int"></div>
                </div>
                <div class="form-group">
                    <label>Touch Threshold</label><input type="number" name="touch_thresh" id="touch_thresh">
                </div>

                <div class="section-title">Layer 1 (Time + Env)</div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Indoor (Sensor)</label><input type="color" name="l1_amb_col" id="l1_amb_col"></div>
                    <div class="form-group" style="flex:1"><label>Outdoor (MQTT)</label><input type="color" name="l1_out_col" id="l1_out_col"></div>
                    <div class="form-group" style="flex:1"><label>Clock Digit</label><input type="color" name="l1_clk_col" id="l1_clk_col"></div>
                </div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Day & Date</label><input type="color" name="l1_dat_col" id="l1_dat_col"></div>
                    <div class="form-group" style="flex:1"><label>Vertical Line</label><input type="color" name="l1_lin_col" id="l1_lin_col"></div>
                </div>
                <div class="section-title">Layer 2 (Time Only)</div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Indoor (Sensor)</label><input type="color" name="l2_amb_col" id="l2_amb_col"></div>
                    <div class="form-group" style="flex:1"><label>Outdoor (MQTT)</label><input type="color" name="l2_out_col" id="l2_out_col"></div>
                    <div class="form-group" style="flex:1"><label>Clock Digit</label><input type="color" name="l2_color" id="l2_color"></div>
                </div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Day & Date</label><input type="color" name="l2_dat_col" id="l2_dat_col"></div>
                    <div class="form-group" style="flex:1"><label>Log Message</label><input type="color" name="log_color" id="log_color"></div>
                    <div class="form-group" style="flex:1"><label>Rolling Msg</label><input type="color" name="scroll_col" id="scroll_col"></div>
                </div>

                <div class="section-title">Layer 3 (Countdown Timer)</div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Start Color</label><input type="color" name="l3_start" id="l3_start"></div>
                    <div class="form-group" style="flex:1"><label>Warn Color</label><input type="color" name="l3_warn" id="l3_warn"></div>
                    <div class="form-group" style="flex:1"><label>End Color</label><input type="color" name="l3_end" id="l3_end"></div>
                </div>
                <div class="form-group">
                    <label>Warning Threshold (%)</label>
                    <input type="number" name="l3_thresh" id="l3_thresh" min="0" max="100">
                </div>
                <div class="form-group">
                    <label>Default Duration</label>
                    <div class="flex-row">
                        <div style="flex:1;"><label style="font-size:12px; color:var(--text-muted); margin-bottom:4px; display:block;">Minutes</label><input type="number" id="l3_def_cnt_min" placeholder="0" min="0"></div>
                        <div style="flex:1;"><label style="font-size:12px; color:var(--text-muted); margin-bottom:4px; display:block;">Seconds</label><input type="number" id="l3_def_cnt_sec" placeholder="0" min="0" max="59"></div>
                    </div>
                </div>
            </div>
            
            <div id="pins" class="tab-content">
                <div class="section-title">Screen Pins (HUB75)</div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>R1</label><input type="number" name="pin_r1" id="pin_r1"></div>
                    <div class="form-group" style="flex:1"><label>G1</label><input type="number" name="pin_g1" id="pin_g1"></div>
                    <div class="form-group" style="flex:1"><label>B1</label><input type="number" name="pin_b1" id="pin_b1"></div>
                </div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>R2</label><input type="number" name="pin_r2" id="pin_r2"></div>
                    <div class="form-group" style="flex:1"><label>G2</label><input type="number" name="pin_g2" id="pin_g2"></div>
                    <div class="form-group" style="flex:1"><label>B2</label><input type="number" name="pin_b2" id="pin_b2"></div>
                </div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>A</label><input type="number" name="pin_a" id="pin_a"></div>
                    <div class="form-group" style="flex:1"><label>B</label><input type="number" name="pin_b" id="pin_b"></div>
                    <div class="form-group" style="flex:1"><label>C</label><input type="number" name="pin_c" id="pin_c"></div>
                    <div class="form-group" style="flex:1"><label>D</label><input type="number" name="pin_d" id="pin_d"></div>
                    <div class="form-group" style="flex:1"><label>E (-1 = None)</label><input type="number" name="pin_e" id="pin_e"></div>
                </div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>LAT (STB)</label><input type="number" name="pin_lat" id="pin_lat"></div>
                    <div class="form-group" style="flex:1"><label>OE</label><input type="number" name="pin_oe" id="pin_oe"></div>
                    <div class="form-group" style="flex:1"><label>CLK</label><input type="number" name="pin_clk" id="pin_clk"></div>
                </div>

                <div class="section-title">Touch & LDR Pins</div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>Touch Button</label><input type="number" name="pin_touch" id="pin_touch"></div>
                    <div class="form-group" style="flex:1"><label>LDR (Photoresistor)</label><input type="number" name="pin_ldr" id="pin_ldr"></div>
                </div>

                <div class="section-title">I2C Pins</div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>AHT20 SDA</label><input type="number" name="pin_aht_sda" id="pin_aht_sda"></div>
                    <div class="form-group" style="flex:1"><label>AHT20 SCL</label><input type="number" name="pin_aht_scl" id="pin_aht_scl"></div>
                </div>
                <div class="flex-row">
                    <div class="form-group" style="flex:1"><label>BME280 SDA</label><input type="number" name="pin_bme_sda" id="pin_bme_sda"></div>
                    <div class="form-group" style="flex:1"><label>BME280 SCL</label><input type="number" name="pin_bme_scl" id="pin_bme_scl"></div>
                </div>
                <div class="form-group"><label>I2C Interrupt (IRQ)</label><input type="number" name="pin_irq" id="pin_irq"></div>
            </div>

            <button type="submit" id="saveBtn">Save & Reboot</button>
        </form>

        <div id="update" class="tab-content">
            <div class="section-title">OTA Firmware Update</div>
            <p style="font-size: 13px; color: var(--text-muted); margin-bottom: 15px;">Upload a compiled .bin file to update the clock wirelessly.</p>
            <form method="POST" action="/update" enctype="multipart/form-data" id="updateForm" onsubmit="document.getElementById('uploadBtn').innerText = 'Uploading...'">
                <div class="form-group">
                    <input type="file" name="update" accept=".bin" style="padding: 10px; background: rgba(0,0,0,0.2); border: 1px dashed var(--glass-border); border-radius: 12px; width: 100%; color: #fff;">
                </div>
                <button type="submit" id="uploadBtn" style="background: linear-gradient(135deg, #f59e0b, #d97706);">Upload & Flash</button>
            </form>
        </div>

        <div id="timer" class="tab-content">
            <div class="section-title">Live Countdown Control</div>
            <div class="form-group">
                <label>Timer Duration</label>
                <div class="flex-row">
                    <div style="flex:1;"><label style="font-size:12px; color:var(--text-muted); margin-bottom:4px; display:block;">Minutes</label><input type="number" id="live_duration_min" placeholder="0" min="0"></div>
                    <div style="flex:1;"><label style="font-size:12px; color:var(--text-muted); margin-bottom:4px; display:block;">Seconds</label><input type="number" id="live_duration_sec" placeholder="0" min="0" max="59"></div>
                </div>
            </div>
            <div class="btn-group">
                <button type="button" onclick="apiCall('start')">Start</button>
                <button type="button" class="btn-stop" onclick="apiCall('stop')">Pause</button>
                <button type="button" style="background: linear-gradient(135deg, #10b981, #059669);" onclick="apiCall('resume')">Resume</button>
                <button type="button" class="btn-reset" onclick="apiCall('reset')">Reset</button>
            </div>
        </div>
        
        <div id="about" class="tab-content">
            <div class="section-title">System Information</div>
            <div style="background: rgba(0,0,0,0.2); padding: 15px; border-radius: 12px; font-size: 14px; line-height: 1.8;">
                <div><strong style="color: var(--text-muted);">Firmware Version:</strong> <span id="info_fw"></span></div>
                <div><strong style="color: var(--text-muted);">Build Date:</strong> <span id="info_build"></span></div>
                <div><strong style="color: var(--text-muted);">Free Heap:</strong> <span id="info_heap"></span></div>
                <div><strong style="color: var(--text-muted);">SDK Version:</strong> <span id="info_sdk"></span></div>
            </div>
            
            <div class="section-title">Build Environment</div>
            <div style="background: rgba(0,0,0,0.2); padding: 15px; border-radius: 12px; font-size: 13px; line-height: 1.6; color: var(--text-muted);">
                &bull; PlatformIO Core v6.1.19<br>
                &bull; Platform Espressif 32 v55.3.311<br>
                &bull; Framework Arduino ESP32 v3.3.11
            </div>

            <div class="section-title">Libraries Used</div>
            <div style="background: rgba(0,0,0,0.2); padding: 15px; border-radius: 12px; font-size: 13px; line-height: 1.6; color: var(--text-muted);">
                &bull; ESP32 HUB75 LED MATRIX PANEL DMA Display v3.0.15<br>
                &bull; Adafruit GFX Library v1.12.6<br>
                &bull; Adafruit BusIO v1.17.4<br>
                &bull; PubSubClient v2.8.0<br>
                &bull; ArduinoJson v6.21.6<br>
                &bull; ESPNtpClient v0.2.7<br>
                &bull; ESP32httpUpdate v2.1.145<br>
                &bull; Adafruit BME280 Library v2.3.0<br>
                &bull; Adafruit AHTX0 v2.0.6
            </div>

            <div class="section-title">Hardware & Source Code</div>
            <div style="background: rgba(0,0,0,0.2); padding: 15px; border-radius: 12px; font-size: 13px; line-height: 1.6; color: var(--text-muted);">
                <strong>Original Project:</strong> <a href="https://github.com/bogd/esp32-morphing-clock.git" target="_blank" style="color: var(--primary); text-decoration: none;">esp32-morphing-clock</a><br>
                <strong>Matrix Shield v1.9:</strong> <a href="https://github.com/hallard/WeMos-Matrix-Shield-DMA" target="_blank" style="color: var(--primary); text-decoration: none;">WeMos-Matrix-Shield-DMA</a><br>
                <strong>Hardware Note:</strong> Designed for HUB75 P4 64x32 LED Matrix
            </div>

            <div class="section-title">License</div>
            <div style="background: rgba(0,0,0,0.2); padding: 15px; border-radius: 12px; font-size: 13px; line-height: 1.6; color: var(--text-muted);">
                <strong>GNU General Public License v3.0</strong><br><br>
                This program is free software: you can redistribute it and/or modify
                it under the terms of the GNU General Public License as published by
                the Free Software Foundation, either version 3 of the License, or
                (at your option) any later version.<br><br>
                <a href="https://www.gnu.org/licenses/gpl-3.0.html" target="_blank" style="color: var(--primary); text-decoration: none;">Read full license</a>
            </div>
        </div>
    </div>

    <script>
        function switchTab(id) {
            document.querySelectorAll('.tab, .tab-content').forEach(el => el.classList.remove('active'));
            document.querySelector(`.tab[onclick="switchTab('${id}')"]`).classList.add('active');
            document.getElementById(id).classList.add('active');
            document.getElementById('saveBtn').style.display = (id === 'timer' || id === 'update' || id === 'about') ? 'none' : 'block';
        }

        fetch('/settings').then(res => res.json()).then(data => {
            if(data.version) {
                document.getElementById('fw_version').innerText = "v" + data.version;
                document.getElementById('info_fw').innerText = "v" + data.version;
            }
            if(data.build) document.getElementById('info_build').innerText = data.build;
            if(data.heap) document.getElementById('info_heap').innerText = data.heap + " bytes";
            if(data.sdk) document.getElementById('info_sdk').innerText = data.sdk;
            Object.keys(data).forEach(key => {
                const el = document.getElementById(key);
                if(el) el.value = data[key];
            });
            if(data.l3_def_cnt) {
                let dur = parseInt(data.l3_def_cnt);
                document.getElementById('l3_def_cnt_min').value = Math.floor(dur / 60);
                document.getElementById('l3_def_cnt_sec').value = dur % 60;
                document.getElementById('live_duration_min').value = Math.floor(dur / 60);
                document.getElementById('live_duration_sec').value = dur % 60;
            }
        });

        function submitForm(e) {
            e.preventDefault();
            const data = Object.fromEntries(new FormData(e.target));
            let m = parseInt(document.getElementById('l3_def_cnt_min').value) || 0;
            let s = parseInt(document.getElementById('l3_def_cnt_sec').value) || 0;
            data.l3_def_cnt = (m * 60 + s).toString();
            
            document.getElementById('saveBtn').innerText = "Saving...";
            fetch('/save', { method: 'POST', body: JSON.stringify(data) }).then(() => {
                setTimeout(() => window.location.reload(), 3000);
            });
        }

        function apiCall(action) {
            let m = parseInt(document.getElementById('live_duration_min').value) || 0;
            let s = parseInt(document.getElementById('live_duration_sec').value) || 0;
            let duration = m * 60 + s;
            fetch(`/api/countdown?action=${action}&dur=${duration}`, { method: 'POST' });
        }
    </script>
</body>
</html>
)=====";

void handleRoot() { webServer.send(200, "text/html", html_page); }

void handleGetSettings() {
  String json = "{";
  json += "\"wifi_ssid\":\"" + wifi_ssid + "\",";
  json += "\"wifi_pass\":\"" + wifi_password + "\",";
  json += "\"mqtt_server\":\"" + mqtt_server + "\",";
  json += "\"mqtt_port\":\"" + String(mqtt_port) + "\",";
  json += "\"mqtt_user\":\"" + mqtt_username + "\",";
  json += "\"mqtt_pass\":\"" + mqtt_password + "\",";
  json += "\"mqtt_temp\":\"" + mqtt_temp_topic + "\",";
  json += "\"mqtt_humi\":\"" + mqtt_humi_topic + "\",";
  json += "\"mqtt_pres\":\"" + mqtt_pres_topic + "\",";
  json += "\"mqtt_layout\":\"" + mqtt_layout_topic + "\",";
  json += "\"mqtt_count\":\"" + mqtt_count_topic + "\",";
  json += "\"l1_amb_col\":\"" + l1_amb_color + "\",";
  json += "\"l1_out_col\":\"" + l1_out_color + "\",";
  json += "\"l1_clk_col\":\"" + l1_clock_color + "\",";
  json += "\"l1_dat_col\":\"" + l1_date_color + "\",";
  json += "\"l1_lin_col\":\"" + l1_line_color + "\",";
  json += "\"l2_amb_col\":\"" + l2_amb_color + "\",";
  json += "\"l2_out_col\":\"" + l2_out_color + "\",";
  json += "\"l2_color\":\"" + l2_clock_color + "\",";
  json += "\"l2_dat_col\":\"" + l2_date_color + "\",";
  json += "\"log_color\":\"" + log_msg_color + "\",";
  json += "\"scroll_col\":\"" + scroll_msg_color + "\",";
  json += "\"l3_start\":\"" + l3_color_start + "\",";
  json += "\"l3_warn\":\"" + l3_color_warn + "\",";
  json += "\"l3_end\":\"" + l3_color_end + "\",";
  json += "\"l3_thresh\":\"" + String(l3_yellow_threshold) + "\",";
  json += "\"brightness\":\"" + String(display_brightness) + "\",";
  json += "\"altitude\":\"" + String(altitude_meters) + "\",";
  json += "\"tz\":\"" + timezone_str + "\",";
  json += "\"ntp_srv\":\"" + ntp_server + "\",";
  json += "\"ntp_int\":\"" + String(ntp_refresh_interval) + "\",";
  json += "\"hostname\":\"" + hostname + "\",";
  json += "\"amb_int\":\"" + String(amb_refresh_interval) + "\",";
  json += "\"dead_int\":\"" + String(sensor_dead_interval) + "\",";
  json += "\"touch_thresh\":\"" + String(touch_threshold) + "\",";
  json += "\"l3_def_cnt\":\"" + String(l3_default_countdown) + "\",";
  json += "\"def_layout\":\"" + String(default_layout) + "\",";
  json += "\"act_sensor\":\"" + String(active_sensor) + "\",";
  json += "\"pin_r1\":\"" + String(pin_r1) + "\",";
  json += "\"pin_g1\":\"" + String(pin_g1) + "\",";
  json += "\"pin_b1\":\"" + String(pin_b1) + "\",";
  json += "\"pin_r2\":\"" + String(pin_r2) + "\",";
  json += "\"pin_g2\":\"" + String(pin_g2) + "\",";
  json += "\"pin_b2\":\"" + String(pin_b2) + "\",";
  json += "\"pin_a\":\"" + String(pin_a) + "\",";
  json += "\"pin_b\":\"" + String(pin_b) + "\",";
  json += "\"pin_c\":\"" + String(pin_c) + "\",";
  json += "\"pin_d\":\"" + String(pin_d) + "\",";
  json += "\"pin_e\":\"" + String(pin_e) + "\",";
  json += "\"pin_lat\":\"" + String(pin_lat) + "\",";
  json += "\"pin_oe\":\"" + String(pin_oe) + "\",";
  json += "\"pin_clk\":\"" + String(pin_clk) + "\",";
  json += "\"pin_touch\":\"" + String(pin_touch_button) + "\",";
  json += "\"pin_ldr\":\"" + String(pin_ldr) + "\",";
  json += "\"pin_aht_sda\":\"" + String(pin_aht20_sda) + "\",";
  json += "\"pin_aht_scl\":\"" + String(pin_aht20_scl) + "\",";
  json += "\"pin_bme_sda\":\"" + String(pin_bme280_sda) + "\",";
  json += "\"pin_bme_scl\":\"" + String(pin_bme280_scl) + "\",";
  json += "\"pin_irq\":\"" + String(pin_irq) + "\",";
  json += "\"version\":\"" + String(FIRMWARE_VERSION) + "\",";
  json += "\"build\":\"" + String(__DATE__) + " " + String(__TIME__) + "\",";
  json += "\"heap\":\"" + String(ESP.getFreeHeap()) + "\",";
  json += "\"sdk\":\"" + String(ESP.getSdkVersion()) + "\"";
  json += "}";
  webServer.send(200, "application/json", json);
}

#include "common.h"
#include <ArduinoJson.h>
#include <Update.h>
extern void switchLayout(int newLayout);

void handleCountdownApi() {
  if (!webServer.hasArg("action"))
    return webServer.send(400, "text/plain", "Missing action");
  String action = webServer.arg("action");

  if (action == "start") {
    int dur = webServer.arg("dur").toInt();
    countdownDurationSec = dur;
    countdownTargetMillis = millis() + (dur * 1000);
    countdownPausedRemainingMs = (dur * 1000);
    countdownActive = true;
    // Layout 2 (Countdown internally)
    switchLayout(2);
  } else if (action == "layout1") {
    countdownActive = false;
    switchLayout(1);
  } else if (action == "layout2") {
    countdownActive = false;
    switchLayout(0);
  } else if (action == "layout3") {
    switchLayout(2);
  } else if (action == "stop") {
    if (countdownActive) {
      countdownPausedRemainingMs = (long)countdownTargetMillis - millis();
      // paused
      countdownActive = false;
    }
  } else if (action == "resume") {
    if (!countdownActive && countdownPausedRemainingMs != 0) {
      countdownTargetMillis = millis() + countdownPausedRemainingMs;
      countdownActive = true;
      switchLayout(2);
    }
  } else if (action == "reset") {
    countdownActive = false;
    int dur = webServer.arg("dur").toInt();
    countdownDurationSec = dur;
    countdownPausedRemainingMs = (dur * 1000);
    countdownTargetMillis = millis() + countdownPausedRemainingMs;
    switchLayout(2);
    clockStartingUp = true;
  }
  webServer.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleSave() {
  if (webServer.hasArg("plain") == false)
    return webServer.send(400, "text/plain", "Body not received");

  String body = webServer.arg("plain");
  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc, body))
    return webServer.send(400, "text/plain", "Invalid JSON");

  wifi_ssid = doc["wifi_ssid"].as<String>();
  wifi_password = doc["wifi_pass"].as<String>();
  mqtt_server = doc["mqtt_server"].as<String>();
  mqtt_port = doc["mqtt_port"].as<int>();
  mqtt_username = doc["mqtt_user"].as<String>();
  mqtt_password = doc["mqtt_pass"].as<String>();
  mqtt_temp_topic = doc["mqtt_temp"].as<String>();
  mqtt_humi_topic = doc["mqtt_humi"].as<String>();
  mqtt_pres_topic = doc["mqtt_pres"].as<String>();
  mqtt_layout_topic = doc["mqtt_layout"].as<String>();
  mqtt_count_topic = doc["mqtt_count"].as<String>();

  l1_amb_color = doc["l1_amb_col"].as<String>();
  l1_out_color = doc["l1_out_col"].as<String>();
  l1_clock_color = doc["l1_clk_col"].as<String>();
  l1_date_color = doc["l1_dat_col"].as<String>();
  l1_line_color = doc["l1_lin_col"].as<String>();
  l2_amb_color = doc["l2_amb_col"].as<String>();
  l2_out_color = doc["l2_out_col"].as<String>();
  l2_clock_color = doc["l2_color"].as<String>();
  l2_date_color = doc["l2_dat_col"].as<String>();
  log_msg_color = doc["log_color"].as<String>();
  scroll_msg_color = doc["scroll_col"].as<String>();
  l3_color_start = doc["l3_start"].as<String>();
  l3_color_warn = doc["l3_warn"].as<String>();
  l3_color_end = doc["l3_end"].as<String>();
  l3_yellow_threshold = doc["l3_thresh"].as<int>();
  l3_default_countdown = doc["l3_def_cnt"].as<int>();
  display_brightness = doc["brightness"].as<int>();
  altitude_meters = doc["altitude"].as<int>();
  timezone_str = doc["tz"].as<String>();
  ntp_server = doc["ntp_srv"].as<String>();
  ntp_refresh_interval = doc["ntp_int"].as<int>();
  hostname = doc["hostname"].as<String>();
  amb_refresh_interval = doc["amb_int"].as<int>();
  sensor_dead_interval = doc["dead_int"].as<int>();
  if (doc.containsKey("touch_thresh"))
    touch_threshold = doc["touch_thresh"].as<int>();
  default_layout = doc["def_layout"].as<int>();
  active_sensor = doc["act_sensor"].as<int>();
  pin_r1 = doc["pin_r1"].as<int8_t>();
  pin_g1 = doc["pin_g1"].as<int8_t>();
  pin_b1 = doc["pin_b1"].as<int8_t>();
  pin_r2 = doc["pin_r2"].as<int8_t>();
  pin_g2 = doc["pin_g2"].as<int8_t>();
  pin_b2 = doc["pin_b2"].as<int8_t>();
  pin_a = doc["pin_a"].as<int8_t>();
  pin_b = doc["pin_b"].as<int8_t>();
  pin_c = doc["pin_c"].as<int8_t>();
  pin_d = doc["pin_d"].as<int8_t>();
  pin_e = doc["pin_e"].as<int8_t>();
  pin_lat = doc["pin_lat"].as<int8_t>();
  pin_oe = doc["pin_oe"].as<int8_t>();
  if (doc.containsKey("pin_clk"))
    pin_clk = doc["pin_clk"].as<int8_t>();
  if (doc.containsKey("pin_touch"))
    pin_touch_button = doc["pin_touch"].as<int8_t>();
  if (doc.containsKey("pin_ldr"))
    pin_ldr = doc["pin_ldr"].as<int8_t>();
  if (doc.containsKey("pin_aht_sda"))
    pin_aht20_sda = doc["pin_aht_sda"].as<int8_t>();
  if (doc.containsKey("pin_aht_scl"))
    pin_aht20_scl = doc["pin_aht_scl"].as<int8_t>();
  if (doc.containsKey("pin_bme_sda"))
    pin_bme280_sda = doc["pin_bme_sda"].as<int8_t>();
  if (doc.containsKey("pin_bme_scl"))
    pin_bme280_scl = doc["pin_bme_scl"].as<int8_t>();
  if (doc.containsKey("pin_irq"))
    pin_irq = doc["pin_irq"].as<int8_t>();


  saveSettings();
  webServer.send(200, "application/json", "{\"status\":\"ok\"}");

  // We can't restart immediately or the browser won't receive the HTTP 200 OK.
  // We signal the main loop to restart after 1 second.
  extern bool shouldReboot;
  extern unsigned long rebootTimer;
  shouldReboot = true;
  rebootTimer = millis();
}

void initWebServer() {
  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/settings", HTTP_GET, handleGetSettings);
  webServer.on("/save", HTTP_POST, handleSave);
  webServer.on("/api/countdown", HTTP_POST, handleCountdownApi);

  // Firmware OTA Update endpoint
  webServer.on(
      "/update", HTTP_POST,
      []() {
        webServer.sendHeader("Connection", "close");
        webServer.send(200, "text/plain",
                       (Update.hasError()) ? "UPDATE FAILED"
                                           : "UPDATE SUCCESSFUL, REBOOTING...");
        delay(1000);
        ESP.restart();
      },
      []() {
        HTTPUpload &upload = webServer.upload();
        if (upload.status == UPLOAD_FILE_START) {
          if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          if (Update.write(upload.buf, upload.currentSize) !=
              upload.currentSize) {
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_END) {
          if (Update.end(true)) {
            Serial.printf("Update Success: %u\nRebooting...\n",
                          upload.totalSize);
          } else {
            Update.printError(Serial);
          }
        }
      });

  // Captive portal fallback
  webServer.onNotFound(handleRoot);

  webServer.begin();
}

void initWebConfigAP() {
  setupModeActive = true;
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("MorphClock");

  dnsServer.start(DNS_PORT, "*", apIP);
  initWebServer();
}

bool shouldReboot = false;
unsigned long rebootTimer = 0;

void handleWebServer() {
  webServer.handleClient();

  if (shouldReboot && millis() - rebootTimer > 1000) {
    ESP.restart();
  }
}

void handleWebConfig() {
  if (setupModeActive) {
    dnsServer.processNextRequest();
    handleWebServer();
  }
}
