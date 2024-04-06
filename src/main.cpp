#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <iostream>
#include <ctime>
#include <time.h>

const char* ssid = "roamos_2g";
const char* password = "lettherebesun";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

const String adjectives[] = {"Savage", "Sultan", "Sorcerer", "Scholar", "Sentinel"};

String g_username = "";
int g_streak = 0;
bool g_streak_completed = false;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

bool streakCompletedToday(const String& end) {
  time_t timeinfo;
  time(&timeinfo);
  struct tm* curr_time = localtime(&timeinfo);
  Serial.print("current time: ");
  Serial.println(curr_time->tm_mday);
  Serial.print("last streak time: ");
  Serial.println(end.substring(8, 10).toInt());
  return curr_time->tm_mday == end.substring(8, 10).toInt();
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Inicializando...");

  Serial.begin(115200);
  delay(1000);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Make a GET request to the Duolingo API
    http.begin("https://www.duolingo.com/2017-06-30/users?username=Zach861038");
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Parse JSON response
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          // Extract data from JSON response
          String username = doc["users"][0]["name"];
          int streak = doc["users"][0]["streakData"]["currentStreak"]["length"];
          String streak_end = doc["users"][0]["streakData"]["currentStreak"]["endDate"];
          g_username = username;
          g_streak = streak;
          g_streak_completed = streakCompletedToday(streak_end);
        } else {
          Serial.println("Failed to parse JSON");
        }
      } else {
        Serial.print("HTTP error code: ");
        Serial.println(httpCode);
      }
    } else {
      Serial.println("Failed to connect to server");
    }
    
    http.end();
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Usuario: " + g_username);
  lcd.setCursor(0, 1);
  lcd.print("Racha: ");
  lcd.print(g_streak);

  delay(15000);

  lcd.clear();
  if (g_streak_completed) {
    lcd.setCursor(1, 0);
    lcd.print("Racha Completo");
    lcd.setCursor(0, 1);
    lcd.print("(:");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Racha Incompleto");
    lcd.setCursor(0, 1);
    lcd.print("):");
  }
  delay(15000); // Wait for 15 seconds before making the next request
}
