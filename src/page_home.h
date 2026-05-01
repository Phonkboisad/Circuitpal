#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"
#include "buttons.h"

// ─────────────────────────────────────────────
//  CircuitPal — Home Menu Page
//  Rounded-box list; Prev/Next scroll, Enter selects
// ─────────────────────────────────────────────

namespace PageHome {

// Menu labels (must match enum Page for page-routing)
static const char* const LABELS[] = {
  "Clock",
  "Timer",
  "Sand Timer",
  "Weather",
  "Gif"
};

// Destination page per menu item
static const Page DESTS[] = {
  PAGE_CLOCK,
  PAGE_TIMER,
  PAGE_SANDTIMER,
  PAGE_WEATHER,
  PAGE_GIF
};

static int8_t _sel = 0;  // currently highlighted item
static bool   _entered = false;
static Page   _dest    = PAGE_HOME;

void reset() { _sel = 0; _entered = false; }
bool wasEntered() { return _entered; }
Page destination() { return _dest; }

// ── Item box layout ───────────────────────────
//  3 items visible at once, centred vertically
//  selected item has filled rounded-rect bg

void draw(Adafruit_SH1106G& display) {
  // Handle input
  _entered = false;
  if (btn_prev()) { _sel = (_sel - 1 + MENU_COUNT) % MENU_COUNT; }
  if (btn_next()) { _sel = (_sel + 1) % MENU_COUNT; }
  if (btn_enter()) { _entered = true; _dest = DESTS[_sel]; }

  display.clearDisplay();

  // Title bar
  display.fillRect(0, 0, SCREEN_WIDTH, 12, SH110X_WHITE);
  display.setTextColor(SH110X_BLACK);
  display.setTextSize(1);
  display.setCursor(28, 2);
  display.print("CircuitPal");

  // Draw 3 visible items: _sel-1, _sel, _sel+1
  int16_t startY = 16;
  int16_t itemH  = 15;
  int16_t itemW  = 100;
  int16_t itemX  = (SCREEN_WIDTH - itemW) / 2;

  for (int8_t i = -1; i <= 1; i++) {
    int8_t idx = (_sel + i + MENU_COUNT) % MENU_COUNT;
    int16_t y  = startY + (i + 1) * (itemH + 2);
    bool    active = (i == 0);

    if (active) {
      // Filled box (selected)
      display.fillRoundRect(itemX, y, itemW, itemH, 4, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else {
      // Outlined box
      display.drawRoundRect(itemX, y, itemW, itemH, 4, SH110X_WHITE);
      display.setTextColor(SH110X_WHITE);
    }
    display.setTextSize(1);
    // Centre text inside box
    int16_t tx = itemX + 6;
    int16_t ty = y + 4;
    display.setCursor(tx, ty);
    // Arrow indicator on active
    if (active) display.print("> ");
    display.print(LABELS[idx]);
  }

  // Scrollbar dots at right edge
  int16_t dotX = SCREEN_WIDTH - 6;
  for (int8_t i = 0; i < MENU_COUNT; i++) {
    int16_t dotY = 18 + i * ((SCREEN_HEIGHT - 18) / MENU_COUNT);
    if (i == _sel)
      display.fillCircle(dotX, dotY, 2, SH110X_WHITE);
    else
      display.drawCircle(dotX, dotY, 2, SH110X_WHITE);
  }

  display.display();
}

} // namespace PageHome
