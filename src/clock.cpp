// SPDX-License-Identifier: GPL-3.0-or-later

#include "clock.h"
#include "common.h"
#include "digit.h"
#include "rgb_display.h"
#include "settings.h"
#include <Fonts/TomThumb.h>

/*
Digit digit0(0, 80 - 1 - 11*1, CLOCK_Y, 0xFFFF);
Digit digit1(0, 80 - 1 - 11*2, CLOCK_Y, 0xFFFF);
Digit digit2(0, 80 - 4 - 11*3, CLOCK_Y, 0xFFFF);
Digit digit3(0, 80 - 4 - 11*4, CLOCK_Y, 0xFFFF);
Digit digit4(0, 80 - 7 - 11*5, CLOCK_Y, 0xFFFF);
Digit digit5(0, 80 - 7 - 11*6, CLOCK_Y, 0xFFFF);
*/

// The Y axis starts at the bottom for the MorphingClock library... :(
int currentLayout = 1;
uint16_t prevcd_color = 0;

Digit digit5(0, L2_CLOCK_X,
             PANEL_HEIGHT - L2_CLOCK_Y - 2 * (L2_CLOCK_SEGMENT_HEIGHT)-3,
             0xFFFF);
Digit digit4(0,
             L2_CLOCK_X + (L2_CLOCK_SEGMENT_WIDTH + L2_CLOCK_SEGMENT_SPACING),
             PANEL_HEIGHT - L2_CLOCK_Y - 2 * (L2_CLOCK_SEGMENT_HEIGHT)-3,
             0xFFFF);
Digit digit3(0,
             L2_CLOCK_X +
                 2 * (L2_CLOCK_SEGMENT_WIDTH + L2_CLOCK_SEGMENT_SPACING) + 3,
             PANEL_HEIGHT - L2_CLOCK_Y - 2 * (L2_CLOCK_SEGMENT_HEIGHT)-3,
             0xFFFF);
Digit digit2(0,
             L2_CLOCK_X +
                 3 * (L2_CLOCK_SEGMENT_WIDTH + L2_CLOCK_SEGMENT_SPACING) + 3,
             PANEL_HEIGHT - L2_CLOCK_Y - 2 * (L2_CLOCK_SEGMENT_HEIGHT)-3,
             0xFFFF);
Digit digit1(0,
             L2_CLOCK_X +
                 4 * (L2_CLOCK_SEGMENT_WIDTH + L2_CLOCK_SEGMENT_SPACING) + 6,
             PANEL_HEIGHT - L2_CLOCK_Y - 2 * (L2_CLOCK_SEGMENT_HEIGHT)-3,
             0xFFFF);
Digit digit0(0,
             L2_CLOCK_X +
                 5 * (L2_CLOCK_SEGMENT_WIDTH + L2_CLOCK_SEGMENT_SPACING) + 6,
             PANEL_HEIGHT - L2_CLOCK_Y - 2 * (L2_CLOCK_SEGMENT_HEIGHT)-3,
             0xFFFF);

void applyLayout(int layoutId) {
  currentLayout = layoutId;
  int W, S, H, X, Y;
  if (layoutId == 0) {
    Digit::setDigitSize(L2_CLOCK_SEGMENT_WIDTH, L2_CLOCK_SEGMENT_HEIGHT);
    W = L2_CLOCK_SEGMENT_WIDTH;
    S = L2_CLOCK_SEGMENT_SPACING;
    H = L2_CLOCK_SEGMENT_HEIGHT;
    X = L2_CLOCK_X;
    Y = L2_CLOCK_Y;
  } else if (layoutId == 1) {
    Digit::setDigitSize(L1_CLOCK_SEGMENT_WIDTH, L1_CLOCK_SEGMENT_HEIGHT);
    W = L1_CLOCK_SEGMENT_WIDTH;
    S = L1_CLOCK_SEGMENT_SPACING;
    H = L1_CLOCK_SEGMENT_HEIGHT;
    X = L1_CLOCK_X;
    Y = L1_CLOCK_Y;
  } else {
    Digit::setDigitSize(COUNTDOWN_CLOCK_SEGMENT_WIDTH,
                        COUNTDOWN_CLOCK_SEGMENT_HEIGHT);
    W = COUNTDOWN_CLOCK_SEGMENT_WIDTH;
    S = COUNTDOWN_CLOCK_SEGMENT_SPACING;
    H = COUNTDOWN_CLOCK_SEGMENT_HEIGHT;
    X = COUNTDOWN_CLOCK_X;
    Y = COUNTDOWN_CLOCK_Y;
    // Force color update for digits
    prevcd_color = 0;
  }

  int yOffset = PANEL_HEIGHT - Y - 2 * H - 3;
  digit5.setOffset(X, yOffset);
  digit4.setOffset(X + (W + S), yOffset);
  digit3.setOffset(X + 2 * (W + S) + 3, yOffset);
  digit2.setOffset(X + 3 * (W + S) + 3, yOffset);
  digit1.setOffset(X + 4 * (W + S) + 6, yOffset);
  digit0.setOffset(X + 5 * (W + S) + 6, yOffset);

  // Apply custom colors
  if (layoutId == 0 && l2_clock_color.length() > 0) {
    uint16_t c = hexToRGB565(l2_clock_color);
    digit5.setColor(c);
    digit4.setColor(c);
    digit3.setColor(c);
    digit2.setColor(c);
    digit1.setColor(c);
    digit0.setColor(c);
  } else if (layoutId == 1) {
    digit5.setColor(hexToRGB565(l1_clock_color));
    digit4.setColor(hexToRGB565(l1_clock_color));
    digit3.setColor(hexToRGB565(l1_clock_color));
    digit2.setColor(hexToRGB565(l1_clock_color));
    digit1.setColor(hexToRGB565(l1_clock_color));
    digit0.setColor(hexToRGB565(l1_clock_color));
  }
}

// Digit digit5(0, CLOCK_X,                                                8,
// CLOCK_DIGIT_COLOR); Digit digit4(0, CLOCK_X +
// (CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING),  8, CLOCK_DIGIT_COLOR); Digit
// digit3(0, CLOCK_X+2*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+3,8,
// CLOCK_DIGIT_COLOR); Digit digit2(0,
// CLOCK_X+3*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+3,8,
// CLOCK_DIGIT_COLOR); Digit digit1(0,
// CLOCK_X+4*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+6,8,
// CLOCK_DIGIT_COLOR); Digit digit0(0,
// CLOCK_X+5*(CLOCK_SEGMENT_WIDTH+CLOCK_SEGMENT_SPACING)+6,8,
// CLOCK_DIGIT_COLOR);

int prevss = 0;
int prevmm = 0;
int prevhh = 0;

void displayClock() {
  int hh = timeinfo.tm_hour;
  int mm = timeinfo.tm_min;
  int ss = timeinfo.tm_sec;

  // If we didn't have a previous time. Just draw it without morphing.
  if (clockStartingUp) {
    dma_display->clearScreen();
    digit0.Draw(ss % 10);
    digit1.Draw(ss / 10);
    digit2.Draw(mm % 10);
    digit3.Draw(mm / 10);
    digit4.Draw(hh % 10);
    digit5.Draw(hh / 10);
    uint16_t c = (currentLayout == 0 && l2_clock_color.length() > 0)
                     ? hexToRGB565(l2_clock_color)
                     : hexToRGB565(l1_clock_color);
    digit1.DrawColon(c);
    digit3.DrawColon(c);
    displayDate();
    clockStartingUp = false;
  } else {
    // epoch changes every miliseconds, we only want to draw when digits
    // actually change.
    if (ss != prevss) {
      int s0 = ss % 10;
      int s1 = ss / 10;
      if (s0 != digit0.Value())
        digit0.Morph(s0);
      if (s1 != digit1.Value())
        digit1.Morph(s1);
      prevss = ss;
    }

    if (mm != prevmm) {
      int m0 = mm % 10;
      int m1 = mm / 10;
      if (m0 != digit2.Value())
        digit2.Morph(m0);
      if (m1 != digit3.Value())
        digit3.Morph(m1);
      displayDate();
      prevmm = mm;
    }

    if (hh != prevhh) {
      int h0 = hh % 10;
      int h1 = hh / 10;
      if (h0 != digit4.Value())
        digit4.Morph(h0);
      if (h1 != digit5.Value())
        digit5.Morph(h1);
      prevhh = hh;
    }
  }
}

int prevcd_ss = -1;
int prevcd_mm = -1;
int prevcd_hh = -1;

void displayCountdown() {
  long remainingMs;
  bool isOver = false;

  if (!countdownActive) {
    if (clockStartingUp && countdownPausedRemainingMs <= 0) {
      if (l3_default_countdown > 0) {
        countdownDurationSec = l3_default_countdown;
        countdownPausedRemainingMs = l3_default_countdown * 1000;
        countdownActive = true;
        countdownTargetMillis = millis() + countdownPausedRemainingMs;
      } else {
        dma_display->clearScreen();
        uint16_t c = hexToRGB565(l3_color_start);
        digit0.setColor(c);
        digit1.setColor(c);
        digit2.setColor(c);
        digit3.setColor(c);
        digit4.setColor(c);
        digit5.setColor(c);
        digit0.Draw(0);
        digit1.Draw(0);
        digit2.Draw(0);
        digit3.Draw(0);
        digit4.Draw(0);
        digit5.Draw(0);
        digit1.DrawColon(c);
        digit3.DrawColon(c);
        clockStartingUp = false;
        return;
      }
    }
    remainingMs = countdownPausedRemainingMs;
    if (remainingMs <= 0) {
      isOver = true;
      // Count up
      remainingMs = -remainingMs;
    }
  } else {
    long currentMillis = millis();
    remainingMs = (long)countdownTargetMillis - currentMillis;

    if (remainingMs <= 0) {
      isOver = true;
      // Count up
      remainingMs = -remainingMs;
    }
  }

  long totalSecs;
  if (isOver) {
    totalSecs = remainingMs / 1000;
  } else {
    totalSecs = (remainingMs + 999) / 1000;
  }

  int hh = totalSecs / 3600;
  int mm = (totalSecs % 3600) / 60;
  int ss = totalSecs % 60;

  uint16_t color = hexToRGB565(l3_color_start);
  if (isOver) {
    color = hexToRGB565(l3_color_end);
  } else if (countdownDurationSec > 0 &&
             totalSecs <= (countdownDurationSec * l3_yellow_threshold / 100)) {
    color = hexToRGB565(l3_color_warn);
  }

  if (color != prevcd_color) {
    digit0.setColor(color);
    digit1.setColor(color);
    digit2.setColor(color);
    digit3.setColor(color);
    digit4.setColor(color);
    digit5.setColor(color);
    // force redraw
    clockStartingUp = true;
    prevcd_color = color;
  }

  bool showHours = (hh > 0);
  bool prevShowHours = (prevcd_hh > 0);
  if (showHours != prevShowHours && prevcd_hh != -1) {
    clockStartingUp = true;
  }

  int W = COUNTDOWN_CLOCK_SEGMENT_WIDTH;
  int S = COUNTDOWN_CLOCK_SEGMENT_SPACING;
  int H = COUNTDOWN_CLOCK_SEGMENT_HEIGHT;
  int Y = COUNTDOWN_CLOCK_Y;
  int yOffset = PANEL_HEIGHT - Y - 2 * H - 3;
  // = 11; // Default for 6 digits (unread assignment)
  int X;

  // If we didn't have a previous time. Just draw it without morphing.
  if (clockStartingUp) {
    dma_display->clearScreen();

    if (!showHours) {
      X = 18;
      digit3.setOffset(X, yOffset);
      digit2.setOffset(X + (W + S), yOffset);
      digit1.setOffset(X + 2 * (W + S) + 3, yOffset);
      digit0.setOffset(X + 3 * (W + S) + 3, yOffset);
    } else {
      X = 11;
      digit5.setOffset(X, yOffset);
      digit4.setOffset(X + (W + S), yOffset);
      digit3.setOffset(X + 2 * (W + S) + 3, yOffset);
      digit2.setOffset(X + 3 * (W + S) + 3, yOffset);
      digit1.setOffset(X + 4 * (W + S) + 6, yOffset);
      digit0.setOffset(X + 5 * (W + S) + 6, yOffset);
    }

    if (isOver) {
      int minusY = COUNTDOWN_CLOCK_Y + COUNTDOWN_CLOCK_SEGMENT_HEIGHT;
      dma_display->fillRect(X - 6, PANEL_HEIGHT - minusY - 2, 4, 2,
                            hexToRGB565(l3_color_end));
    }

    digit0.Draw(ss % 10);
    digit1.Draw(ss / 10);
    digit2.Draw(mm % 10);
    digit3.Draw(mm / 10);
    digit1.DrawColon(color);

    if (showHours) {
      digit4.Draw(hh % 10);
      digit5.Draw(hh / 10);
      digit3.DrawColon(color);
    }

    clockStartingUp = false;
    prevcd_ss = ss;
    prevcd_mm = mm;
    prevcd_hh = hh;
  } else {
    if (ss != prevcd_ss) {
      int s0 = ss % 10;
      int s1 = ss / 10;
      if (s0 != digit0.Value()) {
        digit0.Clear();
        digit0.Draw(s0);
      }
      if (s1 != digit1.Value()) {
        digit1.Clear();
        digit1.Draw(s1);
      }
      prevcd_ss = ss;
    }

    if (mm != prevcd_mm) {
      int m0 = mm % 10;
      int m1 = mm / 10;
      if (m0 != digit2.Value()) {
        digit2.Clear();
        digit2.Draw(m0);
      }
      if (m1 != digit3.Value()) {
        digit3.Clear();
        digit3.Draw(m1);
      }
      prevcd_mm = mm;
    }

    if (showHours && hh != prevcd_hh) {
      int h0 = hh % 10;
      int h1 = hh / 10;
      if (h0 != digit4.Value()) {
        digit4.Clear();
        digit4.Draw(h0);
      }
      if (h1 != digit5.Value()) {
        digit5.Clear();
        digit5.Draw(h1);
      }
      prevcd_hh = hh;
    }
  }
}

void displayDate() {
  if (currentLayout == 0) {
    dma_display->fillRect(L2_DATE_X, L2_DOW_Y - 6, L2_DATE_WIDTH,
                          L2_DATE_HEIGHT, 0);
    dma_display->setTextSize(1);
    dma_display->setTextWrap(false);
    dma_display->setTextColor(hexToRGB565(l2_date_color));
    dma_display->setFont(&TomThumb);
    dma_display->setCursor(L2_DOW_X, L2_DOW_Y);
    dma_display->print(&timeinfo, "%a");
    dma_display->setCursor(L2_DATE_X, L2_DATE_Y);
    dma_display->print(&timeinfo, "%d.%m");
    dma_display->setFont(NULL);
  } else {
    dma_display->fillRect(L1_DATE_X, L1_DOW_Y - 6, L1_DATE_WIDTH,
                          L1_DATE_HEIGHT, 0);
    dma_display->setTextSize(1);
    dma_display->setTextWrap(false);
    dma_display->setTextColor(hexToRGB565(l1_date_color));
    dma_display->setFont(&TomThumb);
    dma_display->setCursor(L1_DOW_X, L1_DOW_Y);
    dma_display->print(&timeinfo, "%a");
    dma_display->setCursor(L1_DATE_X, L1_DATE_Y);
    dma_display->print(&timeinfo, "%d.%m");
    dma_display->setFont(NULL);
  }
}
