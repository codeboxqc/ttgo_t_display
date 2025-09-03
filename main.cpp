#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "esp_sleep.h"

// -------------------- Config --------------------
const char* ntpServer = "pool.ntp.org";
const char* timezone = "EST5EDT,M3.2.0,M11.1.0"; // New York time

#define WAKE_HOUR   2
#define WAKE_MINUTE 55
#define RUN_HOUR    3


TFT_eSPI tft = TFT_eSPI();
#define TFT_BL 32
#define TEXT_SIZE 2

const char* ssid     = "class";
const char* password = "NoClass";

// -------------------- Globals --------------------
#define FONT_HEIGHT 8
int maxLines = tft.height() / (FONT_HEIGHT * TEXT_SIZE);
int displayLine = 0;
int lastProcessedDay = -1; // ensures only 1 RSS run per day

// Forward declarations
extern void processRSSFeeds();
void displayMessage(String message, uint16_t color = TFT_WHITE);
bool connectWiFi();
bool syncTime();
bool getTimeFromHTTP();
 void goToDeepSleepUntil(int targetHour, int targetMinute);
bool firstBootGracePeriod(int minutes);

// -------------------- Display --------------------
void displayMessage(String message, uint16_t color) {
  

  if (displayLine >= 6) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    displayLine = 0;
  }

  tft.setTextColor(color, TFT_BLACK);
  tft.println(message);
  Serial.println(message);
  displayLine++;
}


// -------------------- WiFi --------------------
bool connectWiFi() {
  WiFi.begin(ssid, password);
  displayMessage("Connecting WiFi...", TFT_YELLOW);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
    if (attempts % 5 == 0) {
      displayMessage("Attempting... " + String(attempts), TFT_YELLOW);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    displayMessage("WiFi Connected!", TFT_GREEN);
    displayMessage("IP: " + WiFi.localIP().toString(), TFT_GREEN);
    return true;
  } else {
    displayMessage("WiFi Failed!", TFT_RED);
    return false;
  }
}


// -------------------- Time Sync --------------------
bool syncTime() {
  configTzTime(timezone, ntpServer);
  delay(1000);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    displayMessage("Time synced (NTP)", TFT_GREEN);
    return true;
  } else {
    displayMessage("NTP failed, trying HTTP...", TFT_ORANGE);
    return getTimeFromHTTP();
  }
}

// Fallback: get time from worldtimeapi.org
bool getTimeFromHTTP() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/timezone/America/New_York");
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    int idx = payload.indexOf("\"datetime\":\"");
    if (idx > 0) {
      String datetime = payload.substring(idx + 12, idx + 31); // e.g. "2025-09-03T14:22:05"
      struct tm tm;
      if (strptime(datetime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm)) {
        timeval now = { mktime(&tm), 0 };
        settimeofday(&now, NULL);
        displayMessage("Time synced (HTTP)", TFT_GREEN);
        http.end();
        return true;
      }
    }
  }
  displayMessage("HTTP Time failed", TFT_RED);
  http.end();
  return false;
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(TEXT_SIZE);
  tft.setCursor(0, 0);

  displayMessage("RSS to Telegram Bot", TFT_CYAN);
  displayMessage("Version 1.0", TFT_WHITE);
  delay(2000);

  if (!connectWiFi()) {
    displayMessage("No WiFi - Halting", TFT_RED);
    while (true) { delay(1000); yield(); }
  }

  if (!syncTime()) {
    displayMessage("Time Sync Failed!", TFT_RED);
  }
  else {
    // Time sync OK → print the current time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        String msg = "Current Time: " + String(buf);
        displayMessage(msg, TFT_GREEN);
        Serial.println(msg);
        delay(2000);
    }
}

  displayMessage("Ready", TFT_GREEN);
   delay(500);
  displayMessage("DeepSleep in 5", TFT_RED);
  delay(2000);
 
}

// -------------------- Loop --------------------
void loop() {
    static bool firstLoopMessage = true; // Display once on first loop
    static bool graceMessageShown = false;


    

     

    // Get current local time
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        displayMessage("Time not available", TFT_RED);
        delay(60000); // wait 1 minute and retry
        return;
    }

    int hours   = timeinfo.tm_hour;
    int minutes = timeinfo.tm_min;

    // Ensure WiFi
    if (WiFi.status() != WL_CONNECTED) {
        displayMessage("WiFi lost, reconnecting...", TFT_ORANGE);
        connectWiFi();
        if (WiFi.status() != WL_CONNECTED) {
            displayMessage("Reconnect failed, retrying in 5 min", TFT_RED);
            delay(300000); // wait 5 minutes
            return;
        }
    }

    // Run RSS feeds ONCE per day at 03:00
    if (hours == RUN_HOUR && lastProcessedDay != timeinfo.tm_mday) {
        displayMessage("Running RSS feeds (3 AM)", TFT_GREEN);
        processRSSFeeds();
        lastProcessedDay = timeinfo.tm_mday;

        // Go to deep sleep after finishing RSS feeds
        goToDeepSleepUntil(WAKE_HOUR, WAKE_MINUTE);
    }

    // First boot grace period: stay awake for a few minutes
    if (firstBootGracePeriod(5)) { // 5 min grace
        if (!graceMessageShown) {
            displayMessage("First boot: staying awake for 5 min", TFT_BLUE);
            graceMessageShown = true;
        }
        delay(60000); // check every 1 minute during grace
    } else {
        // Normal idle: go to deep sleep until next wake
        displayMessage("Entering deep sleep until next cycle...", TFT_BLUE);
        goToDeepSleepUntil(WAKE_HOUR, WAKE_MINUTE);
    }
}



bool firstBootGracePeriod(int minutes) {
  static unsigned long bootTime = millis(); // store first call
  unsigned long elapsedMs = millis() - bootTime;
  return elapsedMs < (minutes * 60 * 1000UL);
}






///////////////////////////
void goToDeepSleepUntil(int targetHour, int targetMinute) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    displayMessage("No time for sleep calc", TFT_RED);
    return;
  }

  // Current time
  time_t now = mktime(&timeinfo);

  // Target today
  struct tm wakeTm = timeinfo;
  wakeTm.tm_hour = targetHour;
  wakeTm.tm_min  = targetMinute;
  wakeTm.tm_sec  = 0;

  time_t wakeTime = mktime(&wakeTm);

  // If target already passed today, set for tomorrow
  if (wakeTime <= now) {
    wakeTime += 24 * 60 * 60;
  }

  // Calculate seconds to sleep
  time_t sleepSeconds = wakeTime - now;
  uint64_t sleepUs = (uint64_t)sleepSeconds * 1000000ULL;

  displayMessage("Deep sleep until " + String(targetHour) + ":" + 
                 (targetMinute < 10 ? "0" : "") + String(targetMinute),
                 TFT_BLUE);

  esp_sleep_enable_timer_wakeup(sleepUs);
  esp_deep_sleep_start();
}
