# ttgo_t_display

compile with vscode + addon plugin platformfio  open this  project compile!

TENSTAR T-Display ESP32 WiFi And Bluetooth-Compatible Module Development Board 1.14 Inch LCD Control

Window usb to esp32 driver this model
https://github.com/Xinyuan-LilyGO/CH9102_Driver

esp32 ttgo_t_display news feed rss reader


ESP32 RSS to Telegram Bot

Overview
This project runs an ESP32-based RSS-to-Telegram bot with a TFT display. It connects to Wi-Fi, fetches RSS feeds once a day at 3 AM, and uses low sleep to save power.
Hardware: ESP32, TFT display (TFT_eSPI)
Connectivity: Wi-Fi
Time Sync: NTP or HTTP fallback
Power Management: Deep sleep until next wake-up

Features
Wi-Fi auto-connect with retries
Time synchronization using NTP (fallback to HTTP API)
TFT display for status messages
RSS feed processing once per day at 03:00
Deep sleep between cycles to reduce power
Grace period on first boot to allow manual testing
 

Note: Adjust pins in TFT_eSPI User_Setup.h if different.
Installation
Install Arduino IDE or PlatformIO
Install libraries:
TFT_eSPI
WiFi
HTTPClient
Clone this repo and open main.cpp

/////////////////////////////////////////////////
Edit Wi-Fi credentials: (2.4)

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

engine.cpp
// Telegram constants
const char* BOT_TOKEN = "REALDUMMY";
const char* CHAT_ID = "-DUMMY";
/////////////////////////////////////////////////

setup()
Initializes Serial and TFT
Connects to Wi-Fi
Synchronizes time via NTP or HTTP fallback
Displays status on TFT

loop()

Checks local time
Ensures Wi-Fi is connected
Runs RSS feeds once per day at 3 AM
Goes to deep sleep until next wake-up if outside the grace period


Deep Sleep
Function goToDeepSleepUntil(hour, minute):
Calculates seconds until target time
Calls esp_deep_sleep_start()
ESP32 wakes up automatically at next scheduled time


read rss news feed every days at 3am and post it to your telegram bot example 
///////////////////////
https://t.me/RssFido
///////////////////////


![2](https://github.com/user-attachments/assets/5605fd45-6d9e-44be-a513-6b949c4efe95)
![a](https://github.com/user-attachments/assets/8ff18067-e3d8-4957-a017-e76e3c0e47bd)



