#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/Picopixel.h>
#include <Fonts/TomThumb.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

struct DayTime {
  uint8_t hour;
  uint8_t minute;
};

struct PrayerTimings {
  DayTime fajr;
  DayTime dhuhr;
  DayTime asr;
  DayTime maghrib;
  DayTime isha;
  DayTime nextPrayer;
  String nextPrayerName;
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

PrayerTimings prayerTimings;

const char* ssid = "Salam667";
const char* password = "milouda1965$";

const char* prayerServer = "http://api.aladhan.com/v1/calendarByCity/2023/5?city=Montreal&country=Canada";

tm timeinfo;

tm fajrTime;
tm duhrTime;
tm asrTime;
tm maghribTime;
tm ishaTime;


hw_timer_t * timer = NULL;

uint8_t currentDay = 0;
uint8_t currentMonth = 0;

DeserializationError error;
DynamicJsonDocument doc(62770);

String dataReceived = "";

volatile bool isTimeToPray = false;
volatile bool isPrayed = false;

DayTime convertStringToDayTime(const char* timeString){
  DayTime dayTime;
  char hourString[3];
  char minuteString[3];

  hourString[0] = timeString[0];
  hourString[1] = timeString[1];

  minuteString[0] = timeString[3];
  minuteString[1] = timeString[4];

  dayTime.hour = atoi(hourString);
  dayTime.minute = atoi(minuteString);

  return dayTime;
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);

  int httpResponseCode = http.GET();
  
  String payload = ""; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void updatePrayerTimings(){
  if(WiFi.status()== WL_CONNECTED) {
      
    // ----------------- JSON -----------------
    doc.clear();
    
    
    dataReceived = httpGETRequest(prayerServer);

    error = deserializeJson(doc, dataReceived);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());

      return;
    }

    for (JsonObject data_item : doc["data"].as<JsonArray>()) {

      uint8_t day = data_item["date"]["gregorian"]["day"].as<uint8_t>();
      uint8_t month = data_item["date"]["gregorian"]["month"]["number"].as<uint8_t>();

      if (day == currentDay && month == currentMonth) {

        Serial.println("=====================================");
      
        const char* data_item_date_readable = data_item["date"]["readable"];
        Serial.print("Date: ");
        Serial.println(data_item_date_readable);

        const char* data_item_timings_Fajr = data_item["timings"]["Fajr"];
        Serial.print("Fajr: ");
        Serial.println(data_item_timings_Fajr);
        prayerTimings.fajr = convertStringToDayTime(data_item_timings_Fajr);

        const char* data_item_timings_Dhuhr = data_item["timings"]["Dhuhr"];
        Serial.print("Dhuhr: ");
        Serial.println(data_item_timings_Dhuhr);
        prayerTimings.dhuhr = convertStringToDayTime(data_item_timings_Dhuhr);

        const char* data_item_timings_Asr = data_item["timings"]["Asr"];
        Serial.print("Asr: ");
        Serial.println(data_item_timings_Asr);
        prayerTimings.asr = convertStringToDayTime(data_item_timings_Asr);

        const char* data_item_timings_Maghrib = data_item["timings"]["Maghrib"];
        Serial.print("Maghrib: ");
        Serial.println(data_item_timings_Maghrib);
        prayerTimings.maghrib = convertStringToDayTime(data_item_timings_Maghrib);

        const char* data_item_timings_Isha = data_item["timings"]["Isha"];
        Serial.print("Isha: ");
        Serial.println(data_item_timings_Isha);
        prayerTimings.isha = convertStringToDayTime(data_item_timings_Isha);

        Serial.println("=====================================");
        break;
      }      
    }
  }
}


uint64_t getNextPrayerTimeDelay(){
  uint64_t delay = 0;
  
  getLocalTime(&timeinfo);

  getLocalTime(&fajrTime);
  fajrTime.tm_hour = prayerTimings.fajr.hour;
  fajrTime.tm_min = prayerTimings.fajr.minute;
  fajrTime.tm_sec = 0;

  getLocalTime(&duhrTime);
  duhrTime.tm_hour = prayerTimings.dhuhr.hour;
  duhrTime.tm_min = prayerTimings.dhuhr.minute;
  duhrTime.tm_sec = 0;

  getLocalTime(&asrTime);
  asrTime.tm_hour = prayerTimings.asr.hour;
  asrTime.tm_min = prayerTimings.asr.minute;
  asrTime.tm_sec = 0;

  
  getLocalTime(&maghribTime);
  maghribTime.tm_hour = prayerTimings.maghrib.hour;
  maghribTime.tm_min = prayerTimings.maghrib.minute;
  maghribTime.tm_sec = 0;

  getLocalTime(&ishaTime);
  ishaTime.tm_hour = prayerTimings.isha.hour;
  ishaTime.tm_min = prayerTimings.isha.minute;
  ishaTime.tm_sec = 0;

  double diffFajr = difftime(mktime(&fajrTime), mktime(&timeinfo));
  double diffDhuhr = difftime(mktime(&duhrTime), mktime(&timeinfo));
  double diffAsr = difftime(mktime(&asrTime), mktime(&timeinfo));
  double diffMaghrib = difftime(mktime(&maghribTime), mktime(&timeinfo));
  double diffIsha = difftime(mktime(&ishaTime), mktime(&timeinfo));

  if (diffFajr > 0) {
    delay = diffFajr;
    Serial.println("Fajr");
    prayerTimings.nextPrayer = prayerTimings.fajr;
    prayerTimings.nextPrayerName = "Fajr";
  }
  else if (diffDhuhr > 0) {
    delay = diffDhuhr;
    Serial.println("Dhuhr");
    prayerTimings.nextPrayer = prayerTimings.dhuhr;
    prayerTimings.nextPrayerName = "Dhuhr";
  }
  else if (diffAsr > 0) {
    delay = diffAsr;
    Serial.println("Asr");
    prayerTimings.nextPrayer = prayerTimings.asr;
    prayerTimings.nextPrayerName = "Asr";
  }
  else if (diffMaghrib > 0) {
    delay = diffMaghrib;
    Serial.println("Maghrib");
    prayerTimings.nextPrayer = prayerTimings.maghrib;
    prayerTimings.nextPrayerName = "Maghrib";
  }
  else if (diffIsha > 0) {
    delay = diffIsha;
    Serial.println("Isha");
    prayerTimings.nextPrayer = prayerTimings.isha;
    prayerTimings.nextPrayerName = "Isha";
  }
  else {
    fajrTime.tm_mday = timeinfo.tm_mday + 1;
    delay = difftime(mktime(&fajrTime), mktime(&timeinfo));

    Serial.println("Tomorrow");
    prayerTimings.nextPrayer = prayerTimings.fajr;
    prayerTimings.nextPrayerName = "Fajr";
  }

  return delay * 1000000;
}

void IRAM_ATTR prayerAlert() {
  isTimeToPray = true;
}

void IRAM_ATTR prayed() {
  isPrayed = true;
}

void displayNextPrayer(){
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.setCursor(0, 15);
  display.clearDisplay();
  display.println("Next prayer:");

  display.getTextBounds(prayerTimings.nextPrayerName, 0, 0, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, 35);
  display.println(prayerTimings.nextPrayerName); 

  display.setCursor(33, 55);
  display.print(prayerTimings.nextPrayer.hour);
  display.print(":");
  display.println(prayerTimings.nextPrayer.minute);
  display.display();
}

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(500);
  display.clearDisplay();

  display.setFont(&FreeMonoOblique9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);

  WiFi.begin(ssid, password);
  display.println("Connecting");
  display.display();

  while(WiFi.status() != WL_CONNECTED);

  display.println("Connected");
  display.display();

  delay(500);
  display.clearDisplay();
  display.setCursor(0, 20);

  // Config time for Montreal Time Zone
  configTime(-4 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  getLocalTime(&timeinfo);

  currentDay = timeinfo.tm_mday;
  currentMonth = timeinfo.tm_mon + 1;

  display.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  display.display();
  updatePrayerTimings();

  uint64_t delay = getNextPrayerTimeDelay();
  Serial.println(delay);
  display.clearDisplay();

  displayNextPrayer();

  attachInterrupt(GPIO_NUM_2, prayed, FALLING);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &prayerAlert, true);
  timerAlarmWrite(timer, delay, true);
  timerAlarmEnable(timer);
}

void loop() {

  while(!isTimeToPray);
  isTimeToPray = false;

  timerAlarmDisable(timer);
  

  display.clearDisplay();
  display.setCursor(0,10);
  display.println("Time to pray");
  display.display();

  while (!isPrayed);
  isPrayed = false;

  display.clearDisplay();

  getLocalTime(&timeinfo);
  
  updatePrayerTimings();

  uint64_t delay = getNextPrayerTimeDelay();
  Serial.println(delay);

  displayNextPrayer();

  timerAlarmWrite(timer, delay, true);
  timerAlarmEnable(timer);

}
