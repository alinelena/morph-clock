#include "rgb_display.h"
#include "settings.h"
#include <WiFi.h>
#include "clock.h"
#include "driver/gpio.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "common.h"
#include <Fonts/TomThumb.h>

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// From: https://gist.github.com/davidegironi/3144efdc6d67e5df55438cc3cba613c8
uint16_t colorWheel(uint8_t pos) {
  // Max brightness reduced to 85 (1/3rd of 255) to make it less flashy
  if (pos < 85) {
    return dma_display->color565(pos, 85 - pos, 0);
  } else if (pos < 170) {
    pos -= 85;
    return dma_display->color565(85 - pos, 0, pos);
  } else {
    pos -= 170;
    return dma_display->color565(0, pos, 85 - pos);
  }
}

void display_init() {
  HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN,  B1_PIN, R2_PIN, G2_PIN,
                                   B2_PIN, A_PIN,   B_PIN,  C_PIN,  D_PIN,
                                   E_PIN,  LAT_PIN, OE_PIN, CLK_PIN};

  HUB75_I2S_CFG mxconfig(PANEL_WIDTH,  // Module width
                         PANEL_HEIGHT, // Module height
                         1,            // chain length
                         _pins         // pin mapping
  );
  mxconfig.gpio.e = E_PIN;

  mxconfig.driver = HUB75_I2S_CFG::SHIFTREG;
  mxconfig.clkphase = false;
  mxconfig.min_refresh_rate = 60;
  mxconfig.latch_blanking = 4;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  mxconfig.double_buff = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  dma_display->begin();
}

void logStatusMessage(const char *message) {
  Serial.println(message);
  if (currentLayout == 1 || currentLayout == 2) return;

  // Clear the area inside the borders
  dma_display->fillRect(1, 23, 62, 8, 0);

  dma_display->setTextSize(1); // size 1 == 8 pixels high
  dma_display->setTextWrap(
      false); // Don't wrap at end of line - will do ourselves

  dma_display->setFont(&TomThumb);
  dma_display->setCursor(
      1, 23 + 6); // Baseline at Y=29, keeping it off the bottom border

  dma_display->setTextColor(hexToRGB565(log_msg_color));
  dma_display->print(message);
  dma_display->setFont(NULL);

  messageDisplayMillis = millis();
  logMessageActive = true;
}

void logStatusMessage(String message) {
  Serial.println(message);
  if (currentLayout == 1 || currentLayout == 2) return;

  // Clear the area inside the borders
  dma_display->fillRect(1, 23, 62, 8, 0);

  dma_display->setTextSize(1); // size 1 == 8 pixels high
  dma_display->setTextWrap(
      false); // Don't wrap at end of line - will do ourselves

  dma_display->setFont(&TomThumb);
  dma_display->setCursor(
      1, 23 + 6); // Baseline at Y=29, keeping it off the bottom border

  dma_display->setTextColor(hexToRGB565(log_msg_color));
  dma_display->print(message);
  dma_display->setFont(NULL);

  messageDisplayMillis = millis();
  logMessageActive = true;
}

void clearStatusMessage() {
  if (currentLayout != 1 && currentLayout != 2) {
    dma_display->fillRect(1, 23, 62, 8, 0);
  }
  logMessageActive = false;
}

extern uint8_t rainbowWheelval;

void drawScrollingInfo() {
  if (logMessageActive || currentLayout == 1 || currentLayout == 2) return;

  static int scrollX = 64;
  static String infoString = "";
  static int16_t textWidth = 0;

  if (infoString == "") {
    infoString = "IP: " + WiFi.localIP().toString() + " | Host: " + WiFi.getHostname();

    int16_t x1, y1;
    uint16_t w, h;
    dma_display->setFont(&TomThumb);
    dma_display->getTextBounds(infoString, 0, 0, &x1, &y1, &w, &h);
    textWidth = w;
  }

  // Clear just the text area (Y=24 to Y=30 -> use Y=23, height 8)
  dma_display->fillRect(1, 23, 62, 8, 0);

  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setTextColor(hexToRGB565(scroll_msg_color));
  dma_display->setFont(&TomThumb);
  dma_display->setCursor(scrollX, 23 + 6);
  dma_display->print(infoString);
  dma_display->setFont(NULL);

  // Redraw the left and right border pixels for this row to mask the scrolling text
  for (int y = 23; y <= 30; y++) {
    dma_display->drawPixel(0, y, colorWheel((rainbowWheelval + y) & 255));
    dma_display->drawPixel(63, y, colorWheel((rainbowWheelval + y) & 255));
  }

  scrollX--;
  if (scrollX < -textWidth) {
    scrollX = 64; // Reset to right edge
    infoString = ""; // Refresh info in case IP changed
  }
}

void drawTempHumi(int x, int y, int w, int h, uint16_t color, float temp,
                  int humi) {
  dma_display->fillRect(x, y, w, h, 0);
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setTextColor(color);

  dma_display->setFont(&TomThumb);
  dma_display->setCursor(x, y + 6); // TomThumb baseline is around Y+6

  dma_display->printf("%4.1f", temp);

  // Draw the degree symbol dynamically based on where the text ended
  int deg_x = dma_display->getCursorX();
  dma_display->fillRect(deg_x, y + 1, 2, 2, color);

  // Move cursor past the degree symbol
  dma_display->setCursor(deg_x + 3, y + 6);
  dma_display->printf("C %d%%", humi);

  dma_display->setFont(NULL); // Reset to standard font
}

void draw_hPa(int x, int y, uint16_t color) {
  // 'h' (4px tall)
  dma_display->drawPixel(x, y, color);
  dma_display->drawPixel(x, y+1, color);
  dma_display->drawPixel(x, y+2, color);
  dma_display->drawPixel(x+1, y+2, color);
  dma_display->drawPixel(x, y+3, color);
  dma_display->drawPixel(x+2, y+3, color);

  // 'P' (4px tall)
  y += 5; // y = 7
  dma_display->drawPixel(x, y, color);
  dma_display->drawPixel(x+1, y, color);
  dma_display->drawPixel(x, y+1, color);
  dma_display->drawPixel(x+2, y+1, color);
  dma_display->drawPixel(x, y+2, color);
  dma_display->drawPixel(x+1, y+2, color);
  dma_display->drawPixel(x, y+3, color);

  // 'a' (3px tall)
  y += 5; // y = 12
  dma_display->drawPixel(x+1, y, color);
  dma_display->drawPixel(x+2, y, color);
  dma_display->drawPixel(x, y+1, color);
  dma_display->drawPixel(x+2, y+1, color);
  dma_display->drawPixel(x+1, y+2, color);
  dma_display->drawPixel(x+2, y+2, color);
}

void drawWeatherIcon(int x, int y, int pressure) {
  if (pressure > 1020) {
    // Sun
    uint16_t sunColor = dma_display->color565(255, 255, 0); // Yellow
    dma_display->fillRect(x + 3, y + 1, 3, 3, sunColor);
    dma_display->drawPixel(x + 4, y, sunColor); 
    dma_display->drawPixel(x + 4, y + 4, sunColor); 
    dma_display->drawPixel(x + 2, y + 2, sunColor); 
    dma_display->drawPixel(x + 6, y + 2, sunColor); 
    dma_display->drawPixel(x + 1, y, sunColor);
    dma_display->drawPixel(x + 7, y, sunColor);
    dma_display->drawPixel(x + 1, y + 4, sunColor);
    dma_display->drawPixel(x + 7, y + 4, sunColor);
  } else {
    // Cloud
    uint16_t cloudColor = dma_display->color565(180, 200, 255);
    dma_display->drawFastHLine(x + 2, y, 4, cloudColor);
    dma_display->drawFastHLine(x + 1, y + 1, 7, cloudColor);
    dma_display->drawFastHLine(x, y + 2, 9, cloudColor);
    dma_display->drawFastHLine(x, y + 3, 9, cloudColor);
    
    // Rain
    if (pressure < 1005) {
      uint16_t rainColor = dma_display->color565(0, 100, 255);
      dma_display->drawPixel(x + 1, y + 4, rainColor);
      dma_display->drawPixel(x + 4, y + 4, rainColor);
      dma_display->drawPixel(x + 7, y + 4, rainColor);
      dma_display->drawPixel(x, y + 5, rainColor);
      dma_display->drawPixel(x + 3, y + 5, rainColor);
      dma_display->drawPixel(x + 6, y + 5, rainColor);
    }
  }
}

void drawTempHumiStacked(int x, int w, uint16_t color, float temp, int humi) {
  dma_display->fillRect(x, 1, w, 14, 0); // Clear column (Y=1 to 14)
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setTextColor(color);
  dma_display->setFont(&TomThumb);
  
  // Temperature
  dma_display->setCursor(x, L1_TEMP_Y);
  
  float displayTemp = temp;
  bool isNegative = (temp < 0);
  if (isNegative) {
      displayTemp = -temp;
  }
  
  // Print absolute value without padding to save space
  dma_display->printf("%.1f", displayTemp);
  
  // If negative, draw a tiny minus sign above the first digit
  if (isNegative) {
      // Draw a 3-pixel horizontal line above the first digit
      dma_display->drawFastHLine(x, L1_TEMP_Y - 6, 3, color);
  }
  
  int deg_x = dma_display->getCursorX();
  dma_display->fillRect(deg_x, L1_TEMP_Y - 5, 2, 2, color); // Degree symbol
  dma_display->setCursor(deg_x + 3, L1_TEMP_Y);
  dma_display->print("C");
  
  // Humidity
  char humiStr[8];
  sprintf(humiStr, "%d%%", humi);
  int humiLen = strlen(humiStr);
  int humiWidth = humiLen * 4; // TomThumb is 3px + 1px spacing
  int humiOffset = (w - humiWidth) / 2;
  if (humiOffset < 0) humiOffset = 0;
  
  dma_display->setCursor(x + humiOffset, L1_HUMI_Y);
  dma_display->print(humiStr);
  
  dma_display->setFont(NULL);
}

void displayAmbientalData() {
  if (currentLayout == 0) {
    if (active_sensor > 0) {
      drawTempHumi(L2_SENSOR_DATA_X, L2_SENSOR_DATA_Y, SENSOR_AMB_DATA_WIDTH,
                   SENSOR_AMB_DATA_HEIGHT, hexToRGB565(l2_amb_color), sensorAmbTemp,
                   sensorAmbHumi);
    } else {
      dma_display->fillRect(L2_SENSOR_DATA_X, L2_SENSOR_DATA_Y, SENSOR_AMB_DATA_WIDTH, SENSOR_AMB_DATA_HEIGHT, 0);
    }
  } else {
    // Draw layout 1 yellow separators
    dma_display->drawFastVLine(L1_PART2_X - 1, 1, 14, hexToRGB565(l1_line_color)); // Line 1
    dma_display->drawFastVLine(L1_PART3_X - 1, 1, 14, hexToRGB565(l1_line_color)); // Line 2
    
    if (active_sensor > 0) {
      drawTempHumiStacked(L1_PART1_X, L1_PART_WIDTH, hexToRGB565(l1_amb_color), sensorAmbTemp, sensorAmbHumi);
    } else {
      dma_display->fillRect(L1_PART1_X, 1, L1_PART_WIDTH, 14, 0);
      dma_display->setTextSize(1);
      dma_display->setTextColor(SENSOR_ERROR_DATA_COLOR);
      dma_display->setFont(&TomThumb);
      dma_display->setCursor(L1_PART1_X + 2, L1_TEMP_Y);
      dma_display->print("N/A");
      dma_display->setFont(NULL);
    }
  }
}

void displaySensorData() {
  if (currentLayout == 0) {
    if (sensorDead) {
      dma_display->fillRect(L2_SENSOR_DATA_X, L2_SENSOR_DATA_Y, SENSOR_DATA_WIDTH, SENSOR_DATA_HEIGHT, 0);
      dma_display->setTextSize(1);
      dma_display->setTextWrap(false);
      dma_display->setTextColor(SENSOR_ERROR_DATA_COLOR);
      dma_display->setFont(&TomThumb);
      dma_display->setCursor(L2_SENSOR_DATA_X, L2_SENSOR_DATA_Y + 6);
      dma_display->print("No data!");
      dma_display->setFont(NULL);
    } else {
      drawTempHumi(L2_SENSOR_DATA_X, L2_SENSOR_DATA_Y, SENSOR_DATA_WIDTH,
                   SENSOR_DATA_HEIGHT, hexToRGB565(l2_out_color), sensorTemp, sensorHumi);
    }
  } else {
    if (sensorDead) {
      dma_display->fillRect(L1_PART3_X, 1, L1_PART_WIDTH, 14, 0);
      dma_display->setTextSize(1);
      dma_display->setTextWrap(false);
      dma_display->setTextColor(SENSOR_ERROR_DATA_COLOR);
      dma_display->setFont(&TomThumb);
      dma_display->setCursor(L1_PART3_X, L1_TEMP_Y);
      dma_display->print("No data!");
      dma_display->setFont(NULL);
    } else {
      drawTempHumiStacked(L1_PART3_X, L1_PART_WIDTH, hexToRGB565(l1_out_color), sensorTemp, sensorHumi);
    }
    
    // Draw Pressure in Part 2
    dma_display->fillRect(L1_PART2_X, 1, L1_PART_WIDTH, 14, 0);
    if (active_sensor > 0) {
      drawWeatherIcon(L1_PART2_X + 5, 1, sensorMSLP);

      uint16_t pressureColor = (active_sensor == SENSOR_TYPE_BME280) ? hexToRGB565(l1_amb_color) : hexToRGB565(l1_out_color);
      dma_display->setTextSize(1);
      dma_display->setTextWrap(false);
      dma_display->setTextColor(pressureColor);
      dma_display->setFont(&TomThumb);
      dma_display->setCursor(L1_PART2_X + 1, L1_HUMI_Y);
      if (sensorPressure > 9999) {
          dma_display->printf("%d", 9999);
      } else {
          dma_display->printf("%d", sensorPressure);
      }
      dma_display->setFont(NULL);
      draw_hPa(L1_PART2_X + 16, 2, pressureColor);
    } else {
      dma_display->setTextSize(1);
      dma_display->setTextColor(SENSOR_ERROR_DATA_COLOR);
      dma_display->setFont(&TomThumb);
      dma_display->setCursor(L1_PART2_X + 2, L1_TEMP_Y);
      dma_display->print("N/A");
      dma_display->setFont(NULL);
    }
  }
  newSensorData = false;
}

// Simple R/G/B screen fill, for testing displays
void displayTest(int delayMs) {
  dma_display->fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT,
                        dma_display->color565(255, 0, 0));
  delay(delayMs);
  dma_display->fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT,
                        dma_display->color565(0, 255, 0));
  delay(delayMs);
  dma_display->fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT,
                        dma_display->color565(0, 0, 255));
  delay(delayMs);
  dma_display->fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT,
                        dma_display->color565(0, 0, 0));
}

void drawRainbowBorder(uint8_t start_hue) {
  // Top
  for (int x = 0; x < PANEL_WIDTH; x++) {
    dma_display->drawPixel(x, 0, colorWheel((start_hue + x) & 255));
  }
  // Bottom
  for (int x = 0; x < PANEL_WIDTH; x++) {
    dma_display->drawPixel(x, PANEL_HEIGHT - 1,
                           colorWheel((start_hue + x) & 255));
  }
  // Left
  for (int y = 0; y < PANEL_HEIGHT; y++) {
    dma_display->drawPixel(0, y, colorWheel((start_hue + y) & 255));
  }
  // Right
  for (int y = 0; y < PANEL_HEIGHT; y++) {
    dma_display->drawPixel(PANEL_WIDTH - 1, y,
                           colorWheel((start_hue + y) & 255));
  }
}

static uint32_t heart_8x8[64] = {
    0x000000, 0xFF0000, 0xFF0000, 0x000000, 0xFF0000, 0xFF0000, 0x000000, 0x000000,
    0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0x000000,
    0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0x000000,
    0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0x000000,
    0x000000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0x000000, 0x000000,
    0x000000, 0x000000, 0xFF0000, 0xFF0000, 0xFF0000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0xFF0000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
};

static uint16_t color565(uint32_t rgb) {
  return (((rgb >> 16) & 0xF8) << 8) | (((rgb >> 8) & 0xFC) << 3) |
         ((rgb & 0xFF) >> 3);
}

static void drawBitmap(int startx, int starty, int width, int height,
                       uint32_t *bitmap) {
  int counter = 0;
  for (int yy = 0; yy < height; yy++) {
    for (int xx = 0; xx < width; xx++) {
      dma_display->drawPixel(startx + xx, starty + yy,
                             color565(bitmap[counter]));
      counter++;
    }
  }
}

void drawHeartBeat() {
  if (currentLayout == 1 || currentLayout == 2) return;

  static uint8_t hue = 0;
  static uint8_t tick = 0;

  // Slower fade: Update hue every 3 ticks (90ms) = ~23 seconds for full cycle
  if (++tick >= 3) {
    hue++;
    tick = 0;
  }

  // Fade through all rainbow colors
  uint16_t heartColor = colorWheel(hue);
  
  int hbX = (currentLayout == 0) ? L2_HEARTBEAT_X : L1_HEARTBEAT_X;
  int hbY = (currentLayout == 0) ? L2_HEARTBEAT_Y : L1_HEARTBEAT_Y;

  int counter = 0;
  for (int yy = 0; yy < 8; yy++) {
    for (int xx = 0; xx < 8; xx++) {
      if (heart_8x8[counter] != 0) {
        dma_display->drawPixel(hbX + xx, hbY + yy, heartColor);
      } else {
        dma_display->drawPixel(hbX + xx, hbY + yy,
                               0); // Black background
      }
      counter++;
    }
  }
}
