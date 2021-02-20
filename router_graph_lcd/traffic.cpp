#include <stdlib.h>
#include <rpcWiFi.h>
#include <WiFiUdp.h>
#include "traffic.h"
#include <Arduino_SNMP_Manager.h>

// UI Stuff
LV_FONT_DECLARE(lv_font_montserrat_28_compr_az);  // lowercase only.

#define LV_TRAFFIC_TITLE_HEIGHT 50
#define LV_TRAFFIC_TITLE_PAD 10

#define LV_TRAFFIC_WHITE           lv_color_hex(0xffffff)
#define LV_TRAFFIC_LIGHT           lv_color_hex(0xf3f8fe)
#define LV_TRAFFIC_BLUE            lv_color_hex(0x2f3243)
#define LV_TRAFFIC_RX_COLOR        LV_COLOR_GREEN
#define LV_TRAFFIC_TX_COLOR        LV_COLOR_RED

#define LV_TRAFFIC_POLL_RATE_MS   5000   //ms

static lv_style_t style_title;
static lv_style_t style_tx;
static lv_style_t style_rx;
static lv_obj_t * chart;
static lv_chart_series_t * ser_rx;
static lv_chart_series_t * ser_tx;

// SNMP Stuff
IPAddress router(192, 168, 1, 1);
const char *community = "public";
const int snmpVersion = 0; // SNMP Version 1 = 0, SNMP Version 2 = 1
// OIDs
char *oidInOctetsCount32 = ".1.3.6.1.2.1.2.2.1.10.4";  // Counter32 ETH0 IN
char *oidOutOctetsCount32 = ".1.3.6.1.2.1.2.2.1.16.4"; // Counter32 ETH0 OUT
char *oidUptime = ".1.3.6.1.2.1.1.3.0";                // TimeTicks uptime (hundredths of seconds)
static ValueCallback *callbackInOctets;
static ValueCallback *callbackOutOctets;
static ValueCallback *callbackUptime;

static uint32_t inOctetsResponse = 0;
static uint32_t lastInOctets = 0;
static uint32_t outOctetsResponse = 0;
static uint32_t lastOutOctets = 0;
static int uptime = 0;
static int lastUptime = 0;

WiFiUDP udp;     
SNMPManager snmp = SNMPManager(community);
// UDP object used to send and recieve packets
SNMPGet snmpRequest = SNMPGet(community, snmpVersion); // Starts an SMMPGet instance to send requests

static void getSNMP()
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

static void updateChart() {
  if (uptime == lastUptime) { // already applied to chart.
    return;
  }

  if (uptime < lastUptime)  { // Check if device has rebooted which will reset counters
    Serial.println("Uptime < lastUptime. Device restarted?");
    lastUptime = uptime;
    return;
  }

  uint32_t in_bps = floor((inOctetsResponse-lastInOctets)/(uptime-lastUptime))*100*8;
  uint32_t out_bps = floor((outOctetsResponse-lastOutOctets)/(uptime-lastUptime))*100*8;
  lv_chart_set_next(chart, ser_rx, in_bps/1024/1024);  // bps to Mbps
  lv_chart_set_next(chart, ser_tx, out_bps/1024/1024);

  // Update last samples
  lastUptime = uptime;
  lastInOctets = inOctetsResponse;
  lastOutOctets = outOctetsResponse;  
}

static void setup_snmp() {  
  snmp.setUDP(&udp); // give snmp a pointer to the UDP object
  snmp.begin();      // start the SNMP Manager

  // Create a handler for each of the OID  
  callbackInOctets  = snmp.addCounter32Handler(router, oidInOctetsCount32, &inOctetsResponse);
  callbackOutOctets  = snmp.addCounter32Handler(router, oidOutOctetsCount32, &outOctetsResponse);
  callbackUptime  =  snmp.addTimestampHandler(router, oidUptime, &uptime);
}

static void setup_ui() {  
  // Title  
  lv_style_set_text_color(&style_title, LV_STATE_DEFAULT, LV_TRAFFIC_WHITE);
  lv_style_set_text_font(&style_title, LV_STATE_DEFAULT, &lv_font_montserrat_28_compr_az);

  // Rx
  lv_style_set_text_color(&style_rx, LV_STATE_DEFAULT, LV_TRAFFIC_RX_COLOR);
  lv_style_set_text_font(&style_rx, LV_STATE_DEFAULT, &lv_font_montserrat_28_compr_az);

  // Tx
  lv_style_set_text_color(&style_tx, LV_STATE_DEFAULT, LV_TRAFFIC_TX_COLOR);
  lv_style_set_text_font(&style_tx, LV_STATE_DEFAULT, &lv_font_montserrat_28_compr_az);

  // Create Title bar
  lv_obj_t * bar = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_set_size(bar, LV_HOR_RES, LV_TRAFFIC_TITLE_HEIGHT);
  lv_obj_set_style_local_bg_color(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_TRAFFIC_BLUE);

  lv_obj_t * title = lv_label_create(bar, NULL);
  lv_label_set_text(title, "internet");
  lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_LEFT, 20, LV_TRAFFIC_TITLE_PAD);
  lv_obj_add_style(title, LV_LABEL_PART_MAIN, &style_title);

  lv_obj_t * tx = lv_label_create(bar, NULL);
  lv_label_set_text(tx, "tx");
  lv_obj_align(tx, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, LV_TRAFFIC_TITLE_PAD);
  lv_obj_add_style(tx, LV_LABEL_PART_MAIN, &style_tx);

  lv_obj_t * rx = lv_label_create(bar, NULL);
  lv_label_set_text(rx, "rx");
  lv_obj_align(rx, NULL, LV_ALIGN_IN_TOP_RIGHT, -70, LV_TRAFFIC_TITLE_PAD);
  lv_obj_add_style(rx, LV_LABEL_PART_MAIN, &style_rx);
  
  // Create a chart
  chart = lv_chart_create(lv_scr_act(), NULL);

  lv_obj_set_pos(chart, 0, LV_TRAFFIC_TITLE_HEIGHT);
  lv_obj_set_size(chart, LV_HOR_RES, LV_VER_RES - LV_TRAFFIC_TITLE_HEIGHT);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  lv_chart_set_range(chart, 0, 200);

  const char *ytick="200\n150\n100\n50\n0"; // 0 - 200 Mbps
  lv_chart_set_y_tick_texts(chart,ytick, 0, LV_CHART_AXIS_DRAW_LAST_TICK);
  lv_chart_set_y_tick_length(chart, LV_CHART_TICK_LENGTH_AUTO, LV_CHART_TICK_LENGTH_AUTO);

  lv_obj_set_style_local_pad_left(chart,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 60);
  lv_obj_set_style_local_pad_right(chart,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 10);

  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);

  static lv_style_t styleStats;
  lv_style_set_text_color(&styleStats, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(chart, LV_CHART_PART_BG, &styleStats);

  /*Add two data series*/
  ser_rx = lv_chart_add_series(chart, LV_TRAFFIC_RX_COLOR);
  ser_tx = lv_chart_add_series(chart, LV_TRAFFIC_TX_COLOR);

  // Zero the points - probably not required.
  for(int i=0; i<10; i++) {
    ser_rx->points[i] = 0;
    ser_tx->points[i] = 0;
  }
  lv_chart_refresh(chart); /*Required after direct set*/
}

void traffic() {
  setup_ui();
  setup_snmp();
}

void traffic_loop() {
  static unsigned long curMillis;
  static unsigned long startMillis = 0;
  snmp.loop();
  curMillis = millis();
  if (curMillis - startMillis >= LV_TRAFFIC_POLL_RATE_MS) {      
      getSNMP();
      startMillis = curMillis;
  }
  // As soon as we get the response.
  updateChart();
}
