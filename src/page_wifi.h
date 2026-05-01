#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"

// ─────────────────────────────────────────────
//  CircuitPal — WiFi Portal Page
//  Handles: STA connect → AP fallback → web form
//           → NVS save → re-connect → home
// ─────────────────────────────────────────────

namespace PageWifi {

static WebServer  _server(80);
static Preferences _prefs;
static bool       _connected   = false;
static bool       _failed      = false;
static bool       _formPosted  = false;
static String     _newSSID     = "";
static String     _newPass     = "";

// ── HTML portal page ──────────────────────────
static const char PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>CircuitPal Setup</title>
<style>
  body{font-family:sans-serif;background:#111;color:#eee;display:flex;
       justify-content:center;align-items:center;height:100vh;margin:0}
  .box{background:#1e1e1e;border-radius:12px;padding:32px;width:300px;
       box-shadow:0 4px 24px #000}
  h2{text-align:center;margin-bottom:24px;color:#7cf}
  input{width:100%;padding:10px;margin:8px 0 16px;border-radius:6px;
        border:1px solid #444;background:#2a2a2a;color:#fff;box-sizing:border-box}
  button{width:100%;padding:12px;border:none;border-radius:6px;
         background:#2196F3;color:#fff;font-size:16px;cursor:pointer}
  button:hover{background:#1976D2}
</style></head><body>
<div class="box">
  <h2>CircuitPal Setup</h2>
  <form method="POST" action="/save">
    <label>WiFi SSID</label>
    <input name="ssid" type="text" placeholder="Network name" required>
    <label>Password</label>
    <input name="pass" type="password" placeholder="Password">
    <button type="submit">Connect</button>
  </form>
</div>
</body></html>
)rawliteral";

static const char SAVED_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Saved</title>
<style>body{font-family:sans-serif;background:#111;color:#eee;
            display:flex;justify-content:center;align-items:center;height:100vh}
       p{font-size:20px;color:#7cf;text-align:center}</style>
</head><body><p>✓ Credentials saved.<br>Connecting…</p></body></html>
)rawliteral";

// ── Render OLED status ────────────────────────
static void _oledStatus(Adafruit_SH1106G& display, const char* line1,
                         const char* line2 = nullptr, const char* line3 = nullptr) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 8);  display.println(line1);
  if (line2) { display.setCursor(0, 22); display.println(line2); }
  if (line3) { display.setCursor(0, 36); display.println(line3); }
  display.display();
}

// ── Try STA connection ────────────────────────
// Returns true on success, false on timeout
static bool _trySTA(Adafruit_SH1106G& display,
                    const String& ssid, const String& pass, uint16_t timeoutMs = 8000) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  _oledStatus(display, "Connecting to:", ssid.c_str(), "Please wait...");

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > timeoutMs) return false;
    delay(200);
    // Animate dots
    static uint8_t dots = 0;
    display.fillRect(0, 48, 128, 16, SH110X_BLACK);
    display.setCursor(0, 50);
    for (uint8_t i = 0; i <= (dots % 4); i++) display.print(".");
    display.display();
    dots++;
  }
  return true;
}

// ── Open AP + web server ──────────────────────
static void _startPortal(Adafruit_SH1106G& display) {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress ip = WiFi.softAPIP();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);  display.println("Connect to WiFi:");
  display.setCursor(0, 12); display.println(AP_SSID);
  display.setCursor(0, 24); display.println("Password:");
  display.setCursor(0, 36); display.println(AP_PASSWORD);
  display.setCursor(0, 48); display.print("Visit: ");
  display.println(ip);
  display.display();

  _server.on("/", HTTP_GET, []() {
    _server.send_P(200, "text/html", PORTAL_HTML);
  });

  _server.on("/save", HTTP_POST, []() {
    _newSSID = _server.arg("ssid");
    _newPass = _server.arg("pass");
    _server.send_P(200, "text/html", SAVED_HTML);
    _formPosted = true;
  });

  _server.begin();
}

// ── Public API ────────────────────────────────

bool isConnected() { return _connected; }

// Call once — blocks until WiFi is established or portal flow completes.
// Returns true → go to home, false → keep calling tick()
void begin(Adafruit_SH1106G& display) {
  _connected  = false;
  _failed     = false;
  _formPosted = false;

  // Step 1: load saved credentials
  _prefs.begin(NVS_NAMESPACE, true);
  String ssid = _prefs.getString(NVS_KEY_SSID, "");
  String pass = _prefs.getString(NVS_KEY_PASS, "");
  _prefs.end();

  // Step 2: try saved credentials
  if (ssid.length() > 0) {
    if (_trySTA(display, ssid, pass)) {
      _connected = true;
      _oledStatus(display, "Connected!", ("IP: " + WiFi.localIP().toString()).c_str());
      delay(1200);
      return;
    }
    _oledStatus(display, "Connection failed.", "Opening setup...");
    delay(1200);
  }

  // Step 3: open AP portal
  _startPortal(display);
}

// Call in loop() while waiting for portal form submission
// Returns true when ready to move on
bool tick(Adafruit_SH1106G& display) {
  _server.handleClient();

  if (_formPosted) {
    _formPosted = false;
    _server.stop();
    WiFi.softAPdisconnect(true);

    // Save credentials
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putString(NVS_KEY_SSID, _newSSID);
    _prefs.putString(NVS_KEY_PASS, _newPass);
    _prefs.end();

    // Attempt connection
    if (_trySTA(display, _newSSID, _newPass)) {
      _connected = true;
      _oledStatus(display, "Connected!", ("IP: " + WiFi.localIP().toString()).c_str());
      delay(1200);
      return true;
    } else {
      _oledStatus(display, "Connection failed.", "Restart portal...");
      delay(1500);
      // Re-open portal
      _startPortal(display);
    }
  }
  return false;
}

} // namespace PageWifi
