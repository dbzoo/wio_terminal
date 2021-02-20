/* Simplistic text base display of router internet traffic usage.
   SNMP poll to scrape the statistics and display on the Wio Terminal.
   
   Depends on the library; https://github.com/shortbloke/Arduino_SNMP_Manager
*/
#include "rpcWiFi.h"
#include <WiFiUdp.h>
#include <Arduino_SNMP_Manager.h>
#include <math.h>
#include "TFT_eSPI.h"

//************************************
//* Your WiFi info                   *
//************************************
const char *ssid = "yoursid";
const char *password = "yourpassword";
//************************************

//************************************
//* SNMP Device Info                 *
//************************************
IPAddress router(192, 168, 1, 1);
const char *community = "public";
const int snmpVersion = 1; // SNMP Version 1 = 0, SNMP Version 2 = 1
// OIDs
char *oidInOctetsCount32 = ".1.3.6.1.2.1.2.2.1.10.4";  // Counter32 ETH0 IN
char *oidOutOctetsCount32 = ".1.3.6.1.2.1.2.2.1.16.4"; // Counter32 ETH0 OUT
char *oidUptime = ".1.3.6.1.2.1.1.3.0";                // TimeTicks uptime (hundredths of seconds)
//************************************

//************************************
//* Settings                         *
//************************************
int pollInterval = 10000; // delay in milliseconds
char string[50];          // Maximum length of SNMP get response for String values
//************************************

//************************************
//* Initialise                       *
//************************************
// Variables
uint32_t inOctetsResponse = 0;
uint32_t lastInOctets = 0;
uint32_t outOctetsResponse = 0;
uint32_t lastOutOctets = 0;
int uptime = 0;
int lastUptime = 0;

unsigned long pollStart = 0;
unsigned long intervalBetweenPolls = 0;
// SNMP Objects
WiFiUDP udp;                                           // UDP object used to send and recieve packets
SNMPManager snmp = SNMPManager(community);             // Starts an SMMPManager to listen to replies to get-requests
SNMPGet snmpRequest = SNMPGet(community, snmpVersion); // Starts an SMMPGet instance to send requests

TFT_eSPI tft;

// Blank callback pointer for each OID
ValueCallback *callbackInOctets;
ValueCallback *callbackOutOctets;
ValueCallback *callbackUptime;
//************************************

void clearScreen() {  
  tft.fillRect(0, 50, 320, 190, TFT_BLACK); // Except the title
}

void setup()
{
  Serial.begin(115200);
  // uncomment when debugging to we wait until the serial monitor is connectoed.
  //while(!Serial); // Wait for Serial to be ready  

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.fillRoundRect(0, 0, 320, 50, 10, TFT_NAVY);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Internet Monitor", 20, 10 , 1);

  tft.setFreeFont(&FreeSansBold12pt7b);
  tft.setCursor((320 - tft.textWidth("Connecting to Wi-Fi.."))/2, 120);
  tft.print("Connecting to Wi-Fi..");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    WiFi.begin(ssid, password);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
    
  clearScreen();
  String myip = WiFi.localIP().toString();
  int t_size = tft.textWidth("Connected ") + tft.textWidth(myip);
  tft.setCursor((320 - t_size)/2, 120);
  tft.print("Connected ");
  tft.print(myip);
  
  snmp.setUDP(&udp); // give snmp a pointer to the UDP object
  snmp.begin();      // start the SNMP Manager

  callbackInOctets  = snmp.addCounter32Handler(router, oidInOctetsCount32, &inOctetsResponse);
  callbackOutOctets  = snmp.addCounter32Handler(router, oidOutOctetsCount32, &outOctetsResponse);
  callbackUptime  =  snmp.addTimestampHandler(router, oidUptime, &uptime);
}

void loop()
{  
  snmp.loop();
  intervalBetweenPolls = millis() - pollStart;
  if (intervalBetweenPolls >= pollInterval)
  {
    pollStart += pollInterval; // this prevents drift in the delays
    getSNMP();
    doSNMPCalculations(); // Do something with the data collected
  }
}

void tft_speed(uint32_t t, int y) {
    uint32_t o;
    if((o = floor(t/1024/1024)) > 0) {
       tft.drawNumber(o, 120, y);
       tft.drawString(" Mbps", 160, y);
    } else if((o = floor(t/1024)) > 0) {
       tft.drawNumber(o, 120, y);
       tft.drawString(" Kbps", 160, y);
    } else {
       tft.drawNumber(t, 120, y);
       tft.drawString(" bps", 160, y);
    }
}

void doSNMPCalculations()
{
  if (uptime == lastUptime)
  {                 
    if(lastUptime) { // Not our first poll
      Serial.println("Poll did not update data");
      tft.drawString("Poll did not update data", 0, 188);
    } else {
      tft.drawString("Waiting for data", 0, 188);
    }
    return;
  }
  else if (uptime < lastUptime)
  { // Check if device has rebooted which will reset counters
    Serial.println("Uptime < lastUptime. Device restarted?");
    tft.drawString("Device restarted?", 60, 188);
  }
  else
  {
    uint32_t in_bps = floor((inOctetsResponse-lastInOctets)/(uptime-lastUptime))*100*8;
    uint32_t out_bps = floor((outOctetsResponse-lastOutOctets)/(uptime-lastUptime))*100*8;
    
    tft.setTextColor(TFT_WHITE);
    clearScreen();
    tft.drawString("Rx: ", 60, 88);
    tft.drawString("Tx: ", 60, 148);
    tft_speed(in_bps, 88);
    tft_speed(out_bps, 148);
  }
  // Update last samples
  lastUptime = uptime;
  lastInOctets = inOctetsResponse;
  lastOutOctets = outOctetsResponse;
}

void getSNMP()
{
  // Build a SNMP get-request add each OID to the request
  snmpRequest.addOIDPointer(callbackInOctets);
  snmpRequest.addOIDPointer(callbackOutOctets);
  snmpRequest.addOIDPointer(callbackUptime);

  snmpRequest.setIP(WiFi.localIP()); //IP of the arduino
  snmpRequest.setUDP(&udp);
  snmpRequest.setRequestID(rand() % 5555);
  snmpRequest.sendTo(router);
  snmpRequest.clearOIDList();
}
