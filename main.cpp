#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <EEPROM.h>

// -------------------- Config --------------------
const char* ntpServer = "pool.ntp.org";
const char* timezone = "EST5EDT,M3.2.0,M11.1.0"; // New York time

#define RUN_HOUR 3
#define RUN_MINUTE 0
#define WAKE_WINDOW_HOUR 2
#define WAKE_WINDOW_MINUTE 58

TFT_eSPI tft = TFT_eSPI();
#define TFT_BL 32
#define TEXT_SIZE 2

const char* ssid = "*******";
const char* password = "***********";

// -------------------- Globals --------------------
#define FONT_HEIGHT 8
int maxLines = tft.height() / (FONT_HEIGHT * TEXT_SIZE);
int displayLine = 0;
int lastProcessedDay = -1;

// Forward declarations
extern void processRSSFeeds();
void displayMessage(String message, uint16_t color = TFT_WHITE);
bool connectWiFi();
bool syncTime();
bool getTimeFromHTTP();
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

    EEPROM.begin(4);
    lastProcessedDay = EEPROM.read(0);
    if (lastProcessedDay < 0 || lastProcessedDay > 31) lastProcessedDay = -1;

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
    } else {
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
    
}

// -------------------- Loop --------------------
void loop() {
    static bool firstLoopMessage = true;
    static bool graceMessageShown = false;
    static bool wifiEnabled = true;

    // Get current local time
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        displayMessage("Time sync failed, retrying...", TFT_RED);
        int retries = 5;
        while (retries-- > 0 && !getLocalTime(&timeinfo)) {
            if (!wifiEnabled) {
                WiFi.mode(WIFI_STA);
                connectWiFi();
                wifiEnabled = true;
            }
            syncTime();
            delay(1000);
        }
        if (!getLocalTime(&timeinfo)) {
            displayMessage("Time sync failed, retry in 1 min", TFT_RED);
            delay(60000);
            return;
        }
    }

    int hours = timeinfo.tm_hour;
    int minutes = timeinfo.tm_min;
    int seconds = timeinfo.tm_sec;

    // Debug output
    Serial.printf("Time: %02d:%02d:%02d, Day: %d, LastProcessedDay: %d, WiFi: %s\n",
                  hours, minutes, seconds, timeinfo.tm_mday, lastProcessedDay,
                  wifiEnabled ? "Enabled" : "Disabled");

    // Manage WiFi: enable only near RSS window or if needed for sync
    if (hours == WAKE_WINDOW_HOUR && minutes >= WAKE_WINDOW_MINUTE || hours == RUN_HOUR && minutes <= 5) {
        if (!wifiEnabled) {
            WiFi.mode(WIFI_STA);
            if (connectWiFi()) wifiEnabled = true;
        }
    } else if (wifiEnabled) {
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        wifiEnabled = false;
        displayMessage("WiFi disabled to save power", TFT_BLUE);
    }

    // Ensure WiFi if needed
    if (wifiEnabled && WiFi.status() != WL_CONNECTED) {
        displayMessage("WiFi lost, reconnecting...", TFT_ORANGE);
        connectWiFi();
        if (WiFi.status() != WL_CONNECTED) {
            displayMessage("WiFi reconnect failed, retry in 30s", TFT_RED);
            delay(30000);
            return;
        }
    }

    // Run RSS feeds ONCE per day at 16:00–16:05
    if (hours == RUN_HOUR && minutes >= 0 && minutes <= 5 && lastProcessedDay != timeinfo.tm_mday) {
        displayMessage("Running RSS feeds (4 PM)", TFT_GREEN);
        processRSSFeeds();
        lastProcessedDay = timeinfo.tm_mday;
        EEPROM.write(0, lastProcessedDay);
        EEPROM.commit();
    }

    // First boot grace period: stay awake for 6 minutes
    if (firstBootGracePeriod(6)) {
        if (!graceMessageShown) {
            displayMessage("First boot: staying awake for 6 min", TFT_BLUE);
            graceMessageShown = true;
        }
        delay(60000); // Full power during grace period
    }
    // Stay awake near RSS window (15:55–16:05)
    else if (hours == WAKE_WINDOW_HOUR && minutes >= WAKE_WINDOW_MINUTE || hours == RUN_HOUR && minutes <= 5) {
        displayMessage("Staying awake for RSS window", TFT_BLUE);
        delay(30000); // Check every 30 seconds
    }
    // Low-power mode: light sleep outside RSS window
    else {
        displayMessage("Entering light sleep for 30s...", TFT_BLUE);
        Serial.flush();
        esp_sleep_enable_timer_wakeup(30 * 1000000ULL); // 30 seconds
        esp_light_sleep_start();

        digitalWrite(TFT_BL, HIGH); // Restore backlight after wake

 

    }
}

// -------------------- Grace Period --------------------
bool firstBootGracePeriod(int minutes) {
    static unsigned long bootTime = millis();
    unsigned long elapsedMs = millis() - bootTime;
    return elapsedMs < (minutes * 60 * 1000UL); // Correct: milliseconds
}