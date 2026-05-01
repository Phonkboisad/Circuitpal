#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"
#include "buttons.h"
#include "gif_data.h"

namespace PageGif {

static bool     _exitReq    = false;
static bool     _inSubMenu  = true;
static int8_t   _sel        = 0;
static uint8_t  _curFrame   = 0;
static uint32_t _lastFrame  = 0;

void reset() {
  _exitReq   = false;
  _inSubMenu = true;
  _sel       = 0;
  _curFrame  = 0;
}

bool exitRequested() { return _exitReq; }

// ── Sub-menu draw ─────────────────────────────
static void _drawSubMenu(Adafruit_SH1106G& display) {
  display.clearDisplay();

  // Title bar
  display.fillRect(0, 0, SCREEN_WIDTH, 12, SH110X_WHITE);
  display.setTextColor(SH110X_BLACK);
  display.setTextSize(1);
  display.setCursor(46, 2);
  display.print("Gifs");

  int16_t itemW = 90;
  int16_t itemX = (SCREEN_WIDTH - itemW) / 2;
  int16_t itemH = 14;

  for (int8_t i = 0; i < GIF_COUNT; i++) {
    int16_t y = 16 + i * (itemH + 3);
    bool active = (i == _sel);
    if (active) {
      display.fillRoundRect(itemX, y, itemW, itemH, 4, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else {
      display.drawRoundRect(itemX, y, itemW, itemH, 4, SH110X_WHITE);
      display.setTextColor(SH110X_WHITE);
    }
    display.setCursor(itemX + 6, y + 3);
    if (active) display.print("> ");
    display.print(GIFS[i]->name);   // auto‑uses name from descriptor
  }

  display.display();
}

// ── Frame player ──────────────────────────────
static void _drawGif(Adafruit_SH1106G& display) {
  GifDescriptor* gif = GIFS[_sel];

  uint32_t now = millis();
  uint16_t frameDelay = gif->isVideo ? gif->frameDelayMs : gif->delays[_curFrame];
  if (now - _lastFrame > frameDelay) {
    _curFrame = (_curFrame + 1) % gif->frameCount;
    _lastFrame = now;
  }

  display.clearDisplay();
  if (gif->isVideo) {
    display.drawBitmap(0, 0, gif->videoFrames[_curFrame], SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_WHITE);
  } else {
    display.drawBitmap(0, 0, gif->frames[_curFrame], SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_WHITE);
  }
  display.display();
}

void draw(Adafruit_SH1106G& display) {
  _exitReq = false;

  if (_inSubMenu) {
    if (btn_prev()) { _exitReq = true; return; }
    if (btn_next()) { _sel = (_sel + 1) % GIF_COUNT; }
    if (btn_enter()) {
      _inSubMenu = false;
      _curFrame  = 0;
      _lastFrame = millis();
    }
    _drawSubMenu(display);
  } else {
    if (btn_prev()) { _exitReq = true; _inSubMenu = true; return; }
    if (btn_next()) { _sel = (_sel + 1) % GIF_COUNT; _curFrame = 0; }
    btn_enter(); // nothing
    _drawGif(display);
  }
}

} // namespace PageGif
