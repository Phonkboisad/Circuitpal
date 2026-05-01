#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"

// ─────────────────────────────────────────────
//  CircuitPal — Flip-Clock Drawing Primitives
//  Renders a bold segmented / flip-card style
//  digit pair (HH:MM or MM:SS) on SH1106 OLED
// ─────────────────────────────────────────────

// Card dimensions for two-digit group
#define FC_CARD_W   54
#define FC_CARD_H   28
#define FC_DIGIT_W  24
#define FC_DIGIT_H  26
#define FC_GAP       4   // gap between cards
#define FC_COLON_W   8

// Draw a single large digit using thick segments (bitmap-font style)
// x,y = top-left corner of digit cell (FC_DIGIT_W × FC_DIGIT_H)
static void fc_draw_digit(Adafruit_SH1106G& display, int16_t x, int16_t y, uint8_t d) {
  // 7-segment map: segments a-g
  // a=top, b=top-right, c=bot-right, d=bot, e=bot-left, f=top-left, g=mid
  const uint8_t seg[10] = {
    0b1110111, // 0: a b c d e f
    0b0010010, // 1: b c
    0b1101101, // 2: a b g e d
    0b1111001, // 3: a b g c d
    0b0011011, // 4: f g b c
    0b1011011, // 5: a f g c d
    0b1011111, // 6: a f g e c d
    0b1110010, // 7: a b c
    0b1111111, // 8: all
    0b1111011, // 9: a b c d f g
  };
  if (d > 9) return;
  uint8_t s = seg[d];
  uint8_t t = 3; // segment thickness (keep bold)

  int16_t W = FC_DIGIT_W;
  int16_t H = FC_DIGIT_H;
  int16_t mid = y + H / 2;

  // a — top horizontal
  if (s & 0x40) display.fillRect(x+t, y,         W-2*t, t, SH110X_WHITE);
  // b — top-right vertical
  if (s & 0x20) display.fillRect(x+W-t, y+t,     t, H/2-t, SH110X_WHITE);
  // c — bot-right vertical
  if (s & 0x10) display.fillRect(x+W-t, mid,     t, H/2-t, SH110X_WHITE);
  // d — bottom horizontal
  if (s & 0x08) display.fillRect(x+t, y+H-t,     W-2*t, t, SH110X_WHITE);
  // e — bot-left vertical
  if (s & 0x04) display.fillRect(x,    mid,       t, H/2-t, SH110X_WHITE);
  // f — top-left vertical
  if (s & 0x02) display.fillRect(x,    y+t,       t, H/2-t, SH110X_WHITE);
  // g — middle horizontal
  if (s & 0x01) display.fillRect(x+t, mid-t/2,   W-2*t, t, SH110X_WHITE);
}

// Draw a two-digit number (e.g. hours or minutes) as digits only (no card boundaries)
// cx = horizontal centre of the two-digit block
// y  = top of block
static void fc_draw_pair(Adafruit_SH1106G& display, int16_t cx, int16_t y, uint8_t val) {
  uint8_t tens = val / 10;
  uint8_t ones = val % 10;

  int16_t blockW = FC_DIGIT_W * 2 + FC_GAP + 4; // total width of pair
  int16_t x = cx - blockW / 2;

  // Tens digit (no card)
  fc_draw_digit(display, x + 1, y + 1, tens);

  // Ones digit (no card)
  int16_t x2 = x + FC_DIGIT_W + FC_GAP + 2;
  fc_draw_digit(display, x2 + 1, y + 1, ones);
}

// Draw blinking colon at centre screen
static void fc_draw_colon(Adafruit_SH1106G& display, int16_t cx, int16_t y, bool visible) {
  if (!visible) return;
  int16_t dot = 3;
  display.fillRect(cx - dot/2, y + FC_CARD_H/2 - dot - 2, dot, dot, SH110X_WHITE);
  display.fillRect(cx - dot/2, y + FC_CARD_H/2 + 2,       dot, dot, SH110X_WHITE);
}

// Full HH:MM or MM:SS render centred on screen
// topY: top of the clock block
static void fc_render(Adafruit_SH1106G& display, uint8_t left, uint8_t right, bool colon, int16_t topY = 16) {
  int16_t cx = SCREEN_WIDTH / 2;
  fc_draw_pair(display,  cx - FC_COLON_W/2 - FC_CARD_W/2 - 2, topY, left);
  fc_draw_colon(display, cx, topY, colon);
  fc_draw_pair(display,  cx + FC_COLON_W/2 + FC_CARD_W/2 + 2, topY, right);
}

// ─────────────────────────────────────────────
//  New: Clean digital font renderer (big, centered)
//  Use this for large, clean digits without boundaries.
//  textSize parameter lets callers shrink for hourglass page.
// ─────────────────────────────────────────────
static void fc_render_clean(Adafruit_SH1106G& display, uint8_t left, uint8_t right, bool colon, int16_t topY = 16, uint8_t textSize = 4) {
  display.setTextSize(textSize);
  display.setTextColor(SH110X_WHITE);

  // Build string "HH:MM"
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d%c%02d", left, colon ? ':' : ' ', right);

  // Estimate width: default font is 6px wide per char at size 1
  int16_t numChars = 5; // "HH:MM"
  int16_t charW = 6 * textSize;
  int16_t textW = charW * numChars;

  int16_t x = (SCREEN_WIDTH - textW) / 2;
  if (x < 0) x = 0;

  display.setCursor(x, topY);
  display.print(buf);
}
