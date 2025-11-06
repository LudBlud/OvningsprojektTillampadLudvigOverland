/*
File: TillampadProgProjekt1
Author: Ludvig Overland
Date: 2025-10-15
Description: A program that produces the current time and temperature and respectively outputs it on an OLED screen, a servo motor, and a NeoPixel ring.
*/

#include "RTClib.h"    // Tar me nödvändiga bibliotek
#include "U8glib.h"
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

RTC_DS3231 rtc;      //Skriver om viktiga grejor så det it blir jobbigt
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
Servo myservo;

#define LED_PIN 3
#define LED_COUNT 24
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int servoPin = A2;

//Variabler
String formattedTime = "";
float temperature = 0.0;

void setup() {
  Serial.begin(9600);       // Fast serial monitor
  myservo.attach(servoPin); // Attach servo to pin A2
  myservo.write(90);        // Start the servo centered

  strip.begin(); //Initializing commands for the LED Ring
  strip.show();
  strip.setBrightness(50);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC"); // I did not code the rtc, this took the code from "https://randomnerdtutorials.com/esp32-ds3231-real-time-clock-arduino/"
    Serial.flush(); 
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  DateTime now = rtc.now();

  String yearStr = String(now.year(), DEC);
  String monthStr = (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
  String dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC);
  String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC);
  String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
  String secondStr = (now.second() < 10 ? "0" : "") + String(now.second(), DEC);
  String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

  // Update global variables
  formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;
  temperature = rtc.getTemperature();

  Serial.println(formattedTime);
  Serial.print(temperature);
  Serial.println("ºC\n");

// Calling various functions to output data in a visible manner
  oledDraw();
  servoWrite(temperature);
  updateLedRing(now, temperature);  
  delay(1000);
}

void oledDraw() {
  /*
  This function outputs the date, time in hours and minutes and the temperature and curreny day of the week to an OLED screen
  */

  u8g.setFont(u8g_font_unifont);

  // This block of code divides the formatted time from the void loop into the components of: Day, Date and the commas in-between
  int commaIndex = formattedTime.indexOf(',');
  String dayStr = formattedTime.substring(0, commaIndex);                 // "Monday"
  String dateStr = formattedTime.substring(commaIndex + 2, commaIndex + 12); // "2025-10-09"

  //Had some problem with importing the temp variable from earlier so this is a little overkill but it makes the temp output
  String tempStr = String(temperature, 2) + " oC";

  int spaceIndex = formattedTime.indexOf(' ', commaIndex + 2); // find space after date
  String timeStr = formattedTime.substring(spaceIndex + 1, spaceIndex + 6); // "HH:MM"

  u8g.firstPage();
  do {
    // Draw day and date on left
    u8g.drawStr(0, 20, dayStr.c_str());
    u8g.drawStr(0, 40, dateStr.c_str());

    // Draw temperature below
    u8g.drawStr(0, 60, tempStr.c_str());

    // Draw time in top-right corner
    int screenWidth = 128;
    int timeWidth = u8g.getStrWidth(timeStr.c_str());
    u8g.drawStr(screenWidth - timeWidth, 20, timeStr.c_str()); // right aligned
  } while (u8g.nextPage());
}

//Maps the current temperature to a servo angle and outputs it
void servoWrite(float temperature) {
  int servoAngle = map(temperature, 20, 30, 0, 179);
  myservo.write(servoAngle);
}

// Function to control the Led Ring
void updateLedRing(DateTime now, float temperature) {
  int second = now.second();

  // Determine how many LEDs should be lit (1 LED every 2.5 seconds)
  int activeLEDs = second / 2.5;

  // At the start of a new minute, turn all LEDs off
  if (second == 0) {
    strip.clear();
    strip.show();
    return;
  }

  // blue (0,0,255) -> red (255,0,0)
  int redValue = map(temperature, 20, 30, 0, 255);
  int greenValue = 0;
  int blueValue = map(temperature, 20, 30, 255, 0);
  uint32_t color = strip.Color(redValue, greenValue, blueValue);

  strip.clear();
  for (int i = 0; i <= activeLEDs && i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}
