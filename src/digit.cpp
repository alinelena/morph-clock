// SPDX-License-Identifier: GPL-3.0-or-later

#include "digit.h"
#include "common.h"

const byte sA = 0;
const byte sB = 1;
const byte sC = 2;
const byte sD = 3;
const byte sE = 4;
const byte sF = 5;
const byte sG = 6;
int segHeight = L2_CLOCK_SEGMENT_HEIGHT;
int segWidth = L2_CLOCK_SEGMENT_WIDTH;
const uint16_t height = PANEL_HEIGHT - 1;
const uint16_t width = PANEL_WIDTH - 1;

byte digitBits[] = {
    // 0 ABCDEF--
    0b11111100,
    // 1 -BC-----
    0b01100000,
    // 2 AB-DE-G-
    0b11011010,
    // 3 ABCD--G-
    0b11110010,
    // 4 -BC--FG-
    0b01100110,
    // 5 A-CD-FG-
    0b10110110,
    // 6 A-CDEFG-
    0b10111110,
    // 7 ABC-----
    0b11100000,
    // 8 ABCDEFG-
    0b11111110,
    // 9 ABCD_FG-
    0b11110110,
};

// byte sunBitmap[] {
//   B100100100,
//   B010001000,
//   B001110000,
//   B101110100,
//   B001110000,
//   B010001000,
//   B100100100
// };

uint16_t black;

Digit::Digit(byte value, uint16_t xo, uint16_t yo, uint16_t color) {
  _value = value;
  xOffset = xo;
  yOffset = yo;
  _color = color;
}

void Digit::setDigitSize(int w, int h) {
  segWidth = w;
  segHeight = h;
}

void Digit::setOffset(uint16_t xo, uint16_t yo) {
  xOffset = xo;
  yOffset = yo;
}

void Digit::setColor(uint16_t c) { _color = c; }

void Digit::Clear() {
  // 0 is black
  drawFillRect(0, segHeight * 2 + 2, segWidth + 2, segHeight * 2 + 3, 0);
}

byte Digit::Value() { return _value; }
void Digit::drawPixel(uint16_t x, uint16_t y, uint16_t c) {
  dma_display->drawPixel(xOffset + x, height - (y + yOffset), c);
}

void Digit::drawLine(uint16_t x, uint16_t y, uint16_t x2, uint16_t y2,
                     uint16_t c) {
  dma_display->drawLine(xOffset + x, height - (y + yOffset), xOffset + x2,
                        height - (y2 + yOffset), c);
}

void Digit::drawFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         uint16_t c) {
  dma_display->fillRect(xOffset + x, height - (y + yOffset), w, h, c);
}

void Digit::DrawColon(uint16_t c) {
  drawFillRect(-3, segHeight - 1, 2, 2, c);
  drawFillRect(-3, segHeight + 1 + 3, 2, 2, c);
}

void Digit::drawSeg(byte seg) {
  switch (seg) {
  case sA:
    drawLine(1, segHeight * 2 + 2, segWidth, segHeight * 2 + 2, _color);
    break;
  case sB:
    drawLine(segWidth + 1, segHeight * 2 + 1, segWidth + 1, segHeight + 2,
             _color);
    break;
  case sC:
    drawLine(segWidth + 1, 1, segWidth + 1, segHeight, _color);
    break;
  case sD:
    drawLine(1, 0, segWidth, 0, _color);
    break;
  case sE:
    drawLine(0, 1, 0, segHeight, _color);
    break;
  case sF:
    drawLine(0, segHeight * 2 + 1, 0, segHeight + 2, _color);
    break;
  case sG:
    drawLine(1, segHeight + 1, segWidth, segHeight + 1, _color);
    break;
  }
}

void Digit::Draw(byte value) {
  byte pattern = digitBits[value];
  if (bitRead(pattern, 7))
    drawSeg(sA);
  if (bitRead(pattern, 6))
    drawSeg(sB);
  if (bitRead(pattern, 5))
    drawSeg(sC);
  if (bitRead(pattern, 4))
    drawSeg(sD);
  if (bitRead(pattern, 3))
    drawSeg(sE);
  if (bitRead(pattern, 2))
    drawSeg(sF);
  if (bitRead(pattern, 1))
    drawSeg(sG);
  _value = value;
}

void Digit::Morph2() {
  // TWO
  for (int i = 0; i <= segWidth; i++) {
    if (i < segWidth) {
      drawPixel(segWidth - i, segHeight * 2 + 2, _color);
      drawPixel(segWidth - i, segHeight + 1, _color);
      drawPixel(segWidth - i, 0, _color);
    }

    drawLine(segWidth + 1 - i, 1, segWidth + 1 - i, segHeight, black);
    drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
    delay(animSpeed);
  }
}

void Digit::Morph3() {
  // THREE
  for (int i = 0; i <= segWidth; i++) {
    drawLine(0 + i, 1, 0 + i, segHeight, black);
    drawLine(1 + i, 1, 1 + i, segHeight, _color);
    delay(animSpeed);
  }
}

void Digit::Morph4() {
  // FOUR
  for (int i = 0; i < segWidth; i++) {
    // Erase A
    drawPixel(segWidth - i, segHeight * 2 + 2, black);
    // Draw as F
    drawPixel(0, segHeight * 2 + 1 - i, _color);
    // Erase D
    drawPixel(1 + i, 0, black);
    delay(animSpeed);
  }
  // Ensure F is fully drawn
  drawSeg(sF);
}

void Digit::Morph5() {
  // FIVE
  for (int i = 0; i < segWidth; i++) {
    // Erase B
    drawPixel(segWidth + 1, segHeight + 2 + i, black);
    // Draw as A
    drawPixel(segWidth - i, segHeight * 2 + 2, _color);
    // Draw D
    drawPixel(segWidth - i, 0, _color);
    delay(animSpeed);
  }
  drawLine(segWidth + 1, segHeight * 2 + 1, segWidth + 1, segHeight + 2,
           // Ensure B is fully erased
           black);
}

void Digit::Morph6() {
  // SIX
  for (int i = 0; i <= segWidth; i++) {
    // Move C right to left
    drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
    if (i > 0)
      drawLine(segWidth - i + 1, 1, segWidth - i + 1, segHeight, black);
    delay(animSpeed);
  }
}

void Digit::Morph7() {
  // SEVEN
  for (int i = 0; i <= (segWidth + 1); i++) {
    // Move E left to right
    drawLine(0 + i - 1, 1, 0 + i - 1, segHeight, black);
    drawLine(0 + i, 1, 0 + i, segHeight, _color);

    // Move F left to right
    drawLine(0 + i - 1, segHeight * 2 + 1, 0 + i - 1, segHeight + 2, black);
    drawLine(0 + i, segHeight * 2 + 1, 0 + i, segHeight + 2, _color);

    // Erase D and G gradually
    // D
    drawPixel(1 + i, 0, black);
    // G
    drawPixel(1 + i, segHeight + 1, black);
    delay(animSpeed);
  }
}

void Digit::Morph8() {
  // EIGHT
  for (int i = 0; i <= segWidth; i++) {
    // Move B right to left
    drawLine(segWidth - i, segHeight * 2 + 1, segWidth - i, segHeight + 2,
             _color);
    if (i > 0)
      drawLine(segWidth - i + 1, segHeight * 2 + 1, segWidth - i + 1,
               segHeight + 2, black);

    // Move C right to left
    drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
    if (i > 0)
      drawLine(segWidth - i + 1, 1, segWidth - i + 1, segHeight, black);

    // Gradually draw D and G
    if (i < segWidth) {
      // D
      drawPixel(segWidth - i, 0, _color);
      // G
      drawPixel(segWidth - i, segHeight + 1, _color);
    }
    delay(animSpeed);
  }
}

void Digit::Morph9() {
  // NINE
  for (int i = 0; i <= (segWidth + 1); i++) {
    // Move E left to right
    drawLine(0 + i - 1, 1, 0 + i - 1, segHeight, black);
    drawLine(0 + i, 1, 0 + i, segHeight, _color);
    delay(animSpeed);
  }
}

void Digit::Morph0() {
  // ZERO
  for (int i = 0; i <= segWidth; i++) {
    // If 1 to 0, slide B to F and E to C
    if (_value == 1) {
      // slide B to F
      drawLine(segWidth - i, segHeight * 2 + 1, segWidth - i, segHeight + 2,
               _color);
      if (i > 0)
        drawLine(segWidth - i + 1, segHeight * 2 + 1, segWidth - i + 1,
                 segHeight + 2, black);

      // slide E to C
      drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
      if (i > 0)
        drawLine(segWidth - i + 1, 1, segWidth - i + 1, segHeight, black);

      if (i < segWidth)
        // Draw A
        drawPixel(segWidth - i, segHeight * 2 + 2, _color);
      if (i < segWidth)
        // Draw D
        drawPixel(segWidth - i, 0, _color);
    }

    // If 2 to 0, slide B to F and Flow G to C
    if (_value == 2) {
      // slide B to F
      drawLine(segWidth - i, segHeight * 2 + 1, segWidth - i, segHeight + 2,
               _color);
      if (i > 0)
        drawLine(segWidth - i + 1, segHeight * 2 + 1, segWidth - i + 1,
                 segHeight + 2, black);

      // Erase G left to right
      drawPixel(1 + i, segHeight + 1, black);
      if (i < segWidth)
        // Draw C
        drawPixel(segWidth + 1, segHeight + 1 - i, _color);
    }

    // B to F, C to E
    if (_value == 3) {
      // slide B to F
      drawLine(segWidth - i, segHeight * 2 + 1, segWidth - i, segHeight + 2,
               _color);
      if (i > 0)
        drawLine(segWidth - i + 1, segHeight * 2 + 1, segWidth - i + 1,
                 segHeight + 2, black);

      // Move C to E
      drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
      if (i > 0)
        drawLine(segWidth - i + 1, 1, segWidth - i + 1, segHeight, black);

      // Erase G from right to left
      // G
      drawPixel(segWidth - i, segHeight + 1, black);
    }

    // If 5 to 0, we also need to slide F to B
    if (_value == 5) {
      if (i < segWidth) {
        if (i > 0)
          drawLine(1 + i, segHeight * 2 + 1, 1 + i, segHeight + 2, black);
        drawLine(2 + i, segHeight * 2 + 1, 2 + i, segHeight + 2, _color);
      }
    }

    // If 9 or 5 to 0, Flow G into E
    if (_value == 5 || _value == 9) {
      if (i < segWidth)
        drawPixel(segWidth - i, segHeight + 1, black);
      if (i < segWidth)
        drawPixel(0, segHeight - i, _color);
    }
    delay(animSpeed);
  }

  if (_value == 2)
    drawSeg(sC);
  if (_value == 5 || _value == 9)
    drawSeg(sE);
}

void Digit::Morph1() {
  // Zero or two to One
  for (int i = 0; i <= (segWidth + 1); i++) {
    // Move E left to right
    drawLine(0 + i - 1, 1, 0 + i - 1, segHeight, black);
    drawLine(0 + i, 1, 0 + i, segHeight, _color);

    // Move F left to right
    drawLine(0 + i - 1, segHeight * 2 + 1, 0 + i - 1, segHeight + 2, black);
    drawLine(0 + i, segHeight * 2 + 1, 0 + i, segHeight + 2, _color);

    // Gradually Erase A, G, D
    // A
    drawPixel(1 + i, segHeight * 2 + 2, black);
    // D
    drawPixel(1 + i, 0, black);
    // G
    drawPixel(1 + i, segHeight + 1, black);

    delay(animSpeed);
  }
}

void Digit::Morph(byte newValue) {
  switch (newValue) {
  case 2:
    Morph2();
    break;
  case 3:
    Morph3();
    break;
  case 4:
    Morph4();
    break;
  case 5:
    Morph5();
    break;
  case 6:
    Morph6();
    break;
  case 7:
    Morph7();
    break;
  case 8:
    Morph8();
    break;
  case 9:
    Morph9();
    break;
  case 0:
    Morph0();
    break;
  case 1:
    Morph1();
    break;
  }
  _value = newValue;
}
