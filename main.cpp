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
#define WAKE_WINDOW_MINUTE 45

TFT_eSPI tft = TFT_eSPI();
#define TFT_BL 32
#define TEXT_SIZE 2

const char* ssid = "***************************";
const char* password = "***************************";

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
        displayMessage("Synced Ok", TFT_GREEN);
        return true;
    } else {
        displayMessage("NTP failed!", TFT_RED);
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
                displayMessage("Done.", TFT_MAGENTA);
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

    // Always reset to 0 on cold boot
    EEPROM.begin(4);
    lastProcessedDay = 0;
    EEPROM.write(0, lastProcessedDay);
    EEPROM.commit();

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(TEXT_SIZE);
    tft.setCursor(0, 0);

    displayMessage("RssBotGram", TFT_GREENYELLOW);
    displayMessage("V1.0", TFT_VIOLET);
    delay(6000);

    if (!connectWiFi()) {
        displayMessage("No WiFi Halting", TFT_RED);
        while (true) { delay(1000); yield(); }
    }

    if (!syncTime()) {
        displayMessage("Time Failed!", TFT_RED);
    } else {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
            String msg = "" + String(buf);
            displayMessage(msg, TFT_WHITE);
            Serial.println(msg);
            delay(4000);
        }
    }

    displayMessage("Ready", TFT_SILVER);
    delay(500);
     char buffer[16];   
     sprintf(buffer, "Begin AT:%02d ", RUN_HOUR );     
     displayMessage(buffer, TFT_PURPLE);
    
}

// -------------------- Loop --------------------
void loop() {
    static bool firstLoopMessage = true;
    static bool graceMessageShown = false;
    static bool wifiEnabled = false;
     char buffer[16];
     struct tm timeinfo;
    /// int hours = timeinfo.tm_hour;
    // int minutes = timeinfo.tm_min;
    // int seconds = timeinfo.tm_sec;

    // Get current local time
    
    if (!getLocalTime(&timeinfo)) {
        displayMessage("Time sync failed ", TFT_RED);
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
            displayMessage("Time sync failed", TFT_RED);
            delay(60000);
            return;
        }
    }

     //hours = timeinfo.tm_hour;
    // minutes = timeinfo.tm_min;
    // seconds = timeinfo.tm_sec;

    
 

    
    // sprintf(buffer, "%02d:%02d  %02d:%02d", timeinfo.tm_hour ,RUN_HOUR,lastProcessedDay,timeinfo.tm_mday );     
    //  displayMessage(buffer, TFT_BROWN);
    // delay(10000);
    // Run RSS feeds  
   
    if (timeinfo.tm_hour >= RUN_HOUR && lastProcessedDay != timeinfo.tm_mday) {
    displayMessage("Run RSS feeds", TFT_GREEN);
    lastProcessedDay = timeinfo.tm_mday;   // remember day
    EEPROM.write(0, lastProcessedDay);     // save to EEPROM
    EEPROM.commit();
    processRSSFeeds();  // your function
    displayMessage("EndRss.", TFT_SKYBLUE);
    delay(1000);
    displayMessage("<Idle>", TFT_BLUE);
    }

   
    //lastProcessedDay == timeinfo.tm_mday && 
    //if(timeinfo.tm_hour != RUN_HOUR) { 
        //displayMessage(">", TFT_BLUE);
        sprintf(buffer, ">%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min );
        tft.println(buffer);

         WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        wifiEnabled = false;
      
       if(timeinfo.tm_hour == RUN_HOUR && lastProcessedDay == timeinfo.tm_mday) {
        
        
        esp_sleep_enable_timer_wakeup(45 * 60 * 1000000ULL);
        esp_light_sleep_start();
        digitalWrite(TFT_BL, HIGH); // Restore backlight after wake
        Serial.begin(115200);
        delay(1000);
        WiFi.mode(WIFI_STA);
        connectWiFi();
        wifiEnabled = true;
        delay(2000);
    }
       //60 minutes sleep
        else  {
        esp_sleep_enable_timer_wakeup(25 * 60 * 1000000ULL);
        esp_light_sleep_start();
        digitalWrite(TFT_BL, HIGH); // Restore backlight after wake
        Serial.begin(115200);
        delay(1000);
        WiFi.mode(WIFI_STA);
        connectWiFi();
        wifiEnabled = true;
        delay(2000);
        }

       // 15 minutes deep sleep
       
        
     // }


}

 