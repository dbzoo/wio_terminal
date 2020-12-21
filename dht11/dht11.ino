/*
   Display Temperature in DegC and humidity on the WIO terminal LCD
   DHT11 is connected to the grove plug on the bottom right of WIO.
*/
#include <TFT_eSPI.h>
#include <DHT.h>

#define EVENT_WAIT_TIME 2000 // msec

TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
DHT dht(0, DHT11);
// DHT11 has 1.0c precision
// DHT22 has 0.5c precision
const uint8_t precision = 0; // digits after decimal for DegC (DHT22)
long lastEvent = 0;

void handleTemperaturesensor()
{
  unsigned long now = millis();
  if (now - lastEvent < EVENT_WAIT_TIME)
    return;
  lastEvent = now;

  float tempC = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(tempC) || isnan(humidity)) {
    return;                                     // try again next time
  }
  float tempF = dht.convertCtoF(tempC);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawFloat(tempC, precision, 40, 110, 4);
  tft.drawNumber(tempF, 40 , 150, 4);
  tft.drawNumber(humidity, 220 , 110, 4);
}

void setup()
{
  dht.begin();

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.fillRoundRect(0, 0, 320, 50, 10, TFT_NAVY);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("DHT11 Sensor", 20, 10 , 1);

  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextColor(TFT_YELLOW);

  tft.drawString("Temp.", 36, 76 , 1);
  tft.drawRoundRect(36, 100, 80, 80, 5, TFT_WHITE);
  tft.drawString("C", 92, 108, 1);
  tft.drawString("F", 92, 148, 1);

  tft.drawString("Humidity", 206 , 76, 1);
  tft.drawRoundRect(206, 100, 80, 40, 5, TFT_WHITE);
  tft.drawString("%", 262, 108, 1);
}

void loop()
{
  handleTemperaturesensor();
}
