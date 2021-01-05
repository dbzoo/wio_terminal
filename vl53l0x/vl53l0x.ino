/*
 * Sensor connected to the LEFT grove plug on the WIO.
 */
#include <Wire.h>
#include "Adafruit_VL53L0X.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup()
{
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.fillRoundRect(0, 0, 320, 50, 10, TFT_NAVY);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("VL53L0X Sensor", 20, 10 , 1);

  if (!lox.begin()) {
    tft.drawString("Not Found", 10, 40, 1);
    while (1);
  }

  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.drawString("Range", 30, 70 , 1);
  tft.drawRoundRect(30, 120, 218, 100, 5, TFT_WHITE);
  tft.drawString("mm", 160, 150, 1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void loop()
{
  VL53L0X_RangingMeasurementData_t measure;

  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    tft.fillRect(40,130,120,80,TFT_BLACK);
    char str[12];
    (void) sprintf(str, "% 5d", measure.RangeMilliMeter);
    tft.drawString(str, 40, 150, 1);
    delay(50);
  }
}
