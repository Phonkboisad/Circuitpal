#pragma once

// ─────────────────────────────────────────────
//  CircuitPal — Global Configuration
// ─────────────────────────────────────────────


#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define I2C_ADDRESS    0x3C

#define BTN_PREV   25   
#define BTN_NEXT   26   
#define BTN_ENTER  27   


#define DEBOUNCE_MS  50


#define NVS_NAMESPACE  "circuitpal"
#define NVS_KEY_SSID   "ssid"
#define NVS_KEY_PASS   "password"


#define AP_SSID      "CircuitPal-Setup"
#define AP_PASSWORD  "12345678"


#define NTP_SERVER   "pool.ntp.org"
#define GMT_OFFSET    21600   
#define DST_OFFSET    0


#define OWM_API_KEY   "Your_API_KEY_HERE"
#define OWM_CITY      "Your city"
#define OWM_COUNTRY   "Your country"
#define OWM_URL       "http://api.openweathermap.org/data/2.5/weather?q=" OWM_CITY "," OWM_COUNTRY "&appid=" OWM_API_KEY "&units=metric"

#define FLIP_SPEED_MS  60


#define MENU_COUNT   5
#define GIF_COUNT_STATIC    3


enum Page {
  PAGE_INTRO = 0,
  PAGE_WIFI_PORTAL,
  PAGE_HOME,
  PAGE_CLOCK,
  PAGE_TIMER,
  PAGE_SANDTIMER,
  PAGE_WEATHER,
  PAGE_GIF
};
