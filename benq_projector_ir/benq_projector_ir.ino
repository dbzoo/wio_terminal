#include <IRLibSendBase.h>    // First include the send base
#include <IRLib_P01_NEC.h>    
#include <IRLibCombo.h>     // After all protocols, include this
// All of the above automatically creates a universal sending
// class called "IRsend" containing only the protocols you want.
// Now declare an instance of that sender.
#include "TFT_eSPI.h"

IRsend mySender;
TFT_eSPI tft;

#define RIGHT_BUTTON WIO_KEY_A
#define CENTER_BUTTON WIO_KEY_B
#define LEFT_BUTTON WIO_KEY_C
static bool source = true;

void updateScreen() {
  tft.fillScreen(TFT_BLACK);
  // Position text below buttons (roughly)
  tft.drawString("On", 5, 5);
  tft.drawString(source ? "HDMI-1" : "HDMI-2", 65, 5);
  tft.drawString("Off", 170, 5);  
}

void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.setFreeFont(&FreeSansBold12pt7b); 
  updateScreen();
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(CENTER_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
}

void loop() {
  //https://github.com/Sirr-Al-Asrar/BenQ-W1070-IR-remote
  // Also works with HT2050
  if (digitalRead(LEFT_BUTTON) == LOW) { // power on
     mySender.send(NEC,0xCF20D,0);
  }
  if (digitalRead(RIGHT_BUTTON) == LOW) { // power off
     mySender.send(NEC,0xC728D,0);
     delay(250);
     mySender.send(NEC,0xC728D,0);
  }
  if (digitalRead(CENTER_BUTTON) == LOW) { // source
    source = !source;
    mySender.send(NEC,source ? 0xC1AE5 : 0xC9A65,0); // HDMI-1 / HDMI-2
    updateScreen();
  }
  delay(200);
}
