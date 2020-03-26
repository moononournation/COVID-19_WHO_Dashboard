/*******************************************************************************
 * ESP version of WHO Novel Coronavirus (COVID-19) Situation Dashboard
 * Ref.: https://experience.arcgis.com/experience/685d0ace521648f8a5beeeee1b9125cd
 * 
 * Setup steps:
 * 1. Fill your own SSID_NAME, SSID_PASSWORD and URL_TEMPLATE
 * 2. Change your LCD parameters in Arduino_GFX setting
 ******************************************************************************/

/* WiFi settings */
#define SSID_NAME "YourAP"
#define SSID_PASSWORD "PleaseInputYourPasswordHere"

#define UPDATE_INTERVAL 3600000L // in ms, 1 hour

/* URL: WHO Novel Coronavirus (COVID-19) Situation Dashboard */
const char *who_dashboard_url = "https://experience.arcgis.com/experience/685d0ace521648f8a5beeeee1b9125cd";
const char *who_global_url = "https://services.arcgis.com/5T5nSi527N4F7luB/arcgis/rest/services/Cases_by_country_pt_V3/FeatureServer/0/query?where=1%3D1&returnGeometry=false&outStatistics=%5B%7B%22statisticType%22%3A%22sum%22%2C%22onStatisticField%22%3A%22cum_conf%22%2C%22outStatisticFieldName%22%3A%22cum_conf%22%7D%2C%7B%22statisticType%22%3A%22sum%22%2C%22onStatisticField%22%3A%22cum_death%22%2C%22outStatisticFieldName%22%3A%22cum_death%22%7D%5D&cacheHint=true&f=json";
const char *who_adm0_url = "https://services.arcgis.com/5T5nSi527N4F7luB/arcgis/rest/services/Cases_by_country_pt_V3/FeatureServer/0/query?where=ADM0_NAME%3D%27China%27&outFields=cum_conf%2Ccum_death&returnGeometry=false&cacheHint=true&f=json";
const char *who_adm0_new_conf_url = "https://services.arcgis.com/5T5nSi527N4F7luB/ArcGIS/rest/services/COVID_19_HistoricCasesByCountry(pt)View/FeatureServer/0/query?where=ADM0_NAME%3D%27China%27&outFields=NewCase&returnGeometry=false&orderByFields=DateOfDataEntry+desc&resultRecordCount=1&cacheHint=true&f=json";
const char *who_adm1_url = "https://services.arcgis.com/5T5nSi527N4F7luB/arcgis/rest/services/COVID_19_CasesByAdm1(pt)_VIEW/FeatureServer/0/query?where=ADM1_NAME%3D%27HONG+KONG+SAR%27&outFields=new_conf%2Ccum_conf%2Ccum_death&returnGeometry=false&cacheHint=true&f=json";

#define HTTP_TIMEOUT 60000 // in ms, wait a while for server processing
// #define TFT_BL 22

/*******************************************************************************
 * Start of Arduino_GFX setting
 ******************************************************************************/
#include "SPI.h"
#include "Arduino_HWSPI.h"
#include "Arduino_ESP32SPI.h"
#include "Arduino_SWSPI.h"
#include "Arduino_GFX.h"            // Core graphics library
#include "Arduino_Canvas.h"         // Canvas (framebuffer) library
#include "Arduino_Canvas_Indexed.h" // Indexed Color Canvas (framebuffer) library
#include "Arduino_HX8347C.h"        // Hardware-specific library for HX8347C
#include "Arduino_HX8352C.h"        // Hardware-specific library for HX8352C
#include "Arduino_HX8357B.h"        // Hardware-specific library for HX8357B
#include "Arduino_ILI9225.h"        // Hardware-specific library for ILI9225
#include "Arduino_ILI9341.h"        // Hardware-specific library for ILI9341
#include "Arduino_ILI9481_18bit.h"  // Hardware-specific library for ILI9481
#include "Arduino_ILI9486_18bit.h"  // Hardware-specific library for ILI9486
#include "Arduino_SEPS525.h"        // Hardware-specific library for SEPS525
#include "Arduino_SSD1283A.h"       // Hardware-specific library for SSD1283A
#include "Arduino_SSD1331.h"        // Hardware-specific library for SSD1331
#include "Arduino_SSD1351.h"        // Hardware-specific library for SSD1351
#include "Arduino_ST7735.h"         // Hardware-specific library for ST7735
#include "Arduino_ST7789.h"         // Hardware-specific library for ST7789
#include "Arduino_ST7796.h"         // Hardware-specific library for ST7796

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
#define TFT_BL 32
#include "Arduino_ILI9341_M5STACK.h"
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(27 /* DC */, 14 /* CS */, SCK, MOSI, MISO);
Arduino_ILI9341_M5STACK *gfx = new Arduino_ILI9341_M5STACK(bus, 33 /* RST */, 1 /* rotation */);
#elif defined(ARDUINO_ODROID_ESP32)
#define TFT_BL 14
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(21 /* DC */, 5 /* CS */, SCK, MOSI, MISO);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, -1 /* RST */, 3 /* rotation */);
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus,  -1 /* RST */, 1 /* rotation */, true /* IPS */);
#elif defined(ARDUINO_T) // TTGO T-Watch
#define TFT_BL 12
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(27 /* DC */, 5 /* CS */, 18 /* SCK */, 19 /* MOSI */, -1 /* MISO */);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240, 240, 0, 80);
#else /* not a specific hardware */

#if defined(ESP32)
#define TFT_CS 5
// #define TFT_CS -1 // for display without CS pin
#define TFT_DC 16
// #define TFT_DC 27
// #define TFT_DC -1 // for display without DC pin (9-bit SPI)
#define TFT_RST 17
// #define TFT_RST 33
#define TFT_BL 22
#elif defined(ESP8266)
#define TFT_CS 15
#define TFT_DC 5
#define TFT_RST -1
// #define TFT_BL 4
#else
#define TFT_CS 20
#define TFT_DC 19
#define TFT_RST 18
#define TFT_BL 10
#endif

/*
 * Step 1: Initize one databus for your display
*/

// General software SPI
// Arduino_DataBus *bus = new Arduino_SWSPI(TFT_DC, TFT_CS, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */);

// General hardware SPI
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);

// ESP32 hardware SPI, more customizable parameters
// Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);

/*
 * Step 2: Initize one driver for your display
*/

// Canvas (framebuffer)
// Arduino_ST7789 *output_display = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);
// 16-bit color Canvas (240x320 resolution only works for ESP32 with PSRAM)
// Arduino_Canvas *gfx = new Arduino_Canvas(240, 320, output_display);
// Indexed color Canvas, mask_level: 0-2, larger mask level mean less color variation but can have faster index mapping
// Arduino_Canvas_Indexed *gfx = new Arduino_Canvas_Indexed(240, 320, output_display, MAXMASKLEVEL /* mask_level */);

// HX8347C IPS LCD 240x320
// Arduino_HX8347C *gfx = new Arduino_HX8347C(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// HX8352C IPS LCD 240x400
// Arduino_HX8352C *gfx = new Arduino_HX8352C(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// HX8357B IPS LCD 320x480
// Arduino_HX8357B *gfx = new Arduino_HX8357B(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// ILI9225 LCD 176x220
// Arduino_ILI9225 *gfx = new Arduino_ILI9225(bus, TFT_RST);

// ILI9341 LCD 240x320
// Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, TFT_RST, 2  /* rotation */);

// ILI9481 LCD 320x480
// Arduino_ILI9481_18bit *gfx = new Arduino_ILI9481_18bit(bus, TFT_RST);

// ILI9486 LCD 320x480
// Arduino_ILI9486_18bit *gfx = new Arduino_ILI9486_18bit(bus, TFT_RST);

// SEPS525 OLED 160x128
// Arduino_SEPS525 *gfx = new Arduino_SEPS525(bus, TFT_RST);

// SSD1283A OLED 130x130
// Arduino_SSD1283A *gfx = new Arduino_SSD1283A(bus, TFT_RST);

// SSD1331 OLED 96x64
// Arduino_SSD1331 *gfx = new Arduino_SSD1331(bus, TFT_RST);

// SSD1351 OLED 128x128
// Arduino_SSD1351 *gfx = new Arduino_SSD1351(bus, TFT_RST);

// ST7735 LCD
// 1.8" REDTAB 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST);
// 1.8" BLACKTAB 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 1 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */, false /* BGR */);
// 1.8" GREENTAB A 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 1 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.8" GREENTAB B 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 3 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.8" Wide angle LCD 128x160
Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 2 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */, false /* BGR */);
// 1.5" GREENTAB B 128x128
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 128 /* height */, 2 /* col offset 1 */, 3 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.5" GREENTAB C 128x128
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 128 /* height */, 0 /* col offset 1 */, 32 /* row offset 1 */);
// 0.96" IPS LCD 80x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 3 /* rotation */, true /* IPS */, 80 /* width */, 160 /* height */, 26 /* col offset 1 */, 1 /* row offset 1 */, 26 /* col offset 2 */, 1 /* row offset 2 */);

// ST7789 LCD
// 2.4" LCD 240x320
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST);
// 2.4" IPS LCD 240x320
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);
// 1.3"/1.5" square IPS LCD 240x240
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);
// 1.14" IPS LCD 135x240 TTGO T-Display
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 135 /* width */, 240 /* height */, 53 /* col offset 1 */, 40 /* row offset 1 */, 52 /* col offset 2 */, 40 /* row offset 2 */);

// ST7796 LCD
// 4" LCD 320x480
// Arduino_ST7796 *gfx = new Arduino_ST7796(bus, TFT_RST);
// 4" IPS LCD 320x480
// Arduino_ST7796 *gfx = new Arduino_ST7796(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

#endif /* not a specific hardware */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

#include "FreeSans12pt7b.h"
#include "FreeSans18pt7b.h"
#include "FreeSerifBold9pt7b.h"
#include "FreeSerifBold12pt7b.h"
#include "FreeMonoBold9pt7b.h"
#include "FreeMonoBold12pt7b.h"
#include "FreeMonoBold18pt7b.h"

#if defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#else // ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

// HTTPS howto: https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/
const char *arcgis_root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
    "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
    "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
    "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
    "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
    "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
    "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
    "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
    "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
    "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
    "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
    "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
    "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
    "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
    "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
    "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
    "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
    "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
    "+OkuE6N36B9K\n"
    "-----END CERTIFICATE-----\n";

static HTTPClient http;

/* static variables */
static uint32_t last_rss_update_ms = 0;
static uint16_t banner_color;
static uint16_t font_color_01;
static uint16_t font_color_02;
static uint16_t font_color_03;
static uint16_t panel_color_01;
static uint16_t panel_color_02;
static uint16_t panel_color_03;
static uint16_t panel_color_04;
static uint16_t panel_color_05;
static uint16_t panel_color_06;
static int16_t w, h, pw, ph, y_offset;
const GFXfont *f1, *f2, *f3, *f4;
static int16_t fh1, fo1, fw2, fh2, fo2, fw3, fh3, fo3, fw4, fh4, fo4;

String getHttpsReturnStr(const char *url, const char *root_ca)
{
  HTTPClient https;
  String result;

  Serial.printf("[HTTPS] begin...\n");
  https.begin(url, root_ca);

  Serial.printf("[HTTPS] GET...\n");
  int httpCode = https.GET();

  Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
  // HTTP header has been send and Server response header has been handled
  if (httpCode <= 0)
  {
    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    https.end();
    return result;
  }

  if (httpCode != HTTP_CODE_OK)
  {
    Serial.printf("[HTTPS] Not OK!\n");
    https.end();
    return result;
  }

  delay(100);

  result = https.getString();
  https.end();
  return result;
}

void print_middle(int16_t x, int16_t y, int16_t fw, char prefix, int n)
{
  int t = n;
  int d = 0;
  while (t > 1000)
  {
    t /= 1000;
    d += 4;
  }
  while (t)
  {
    t /= 10;
    ++d;
  }
  x += ((pw - (d * fw)) / 2);
  if (prefix)
  {
    x -= fw;
  }
  gfx->setCursor(x, y);
  if (prefix)
  {
    gfx->print(prefix);
  }
  if (n > 1000000)
  {
    gfx->print(n / 1000000);
    gfx->print(',');
    n %= 1000000;
  }
  if (n > 1000)
  {
    gfx->print(n / 1000);
    gfx->print(',');
    n %= 1000;
  }
  gfx->print(n);
}

/**
 * updateFigures
 * @return bool
 *    true if update success
*/
bool updateFigures()
{
  getHttpsReturnStr(who_global_url, arcgis_root_ca);

  String json = getHttpsReturnStr(who_global_url, arcgis_root_ca);
  // Serial.printf("return: %s\n", json.c_str());

  // global confirmed cases count
  int key_idx = json.indexOf("features");
  key_idx = json.indexOf("cum_conf", key_idx);
  int val_start_idx = json.indexOf(':', key_idx) + 1;
  int val_end_idx = json.indexOf(",", val_start_idx);
  int global_cum_conf = json.substring(val_start_idx, val_end_idx).toInt();
  Serial.printf("global_cum_conf: %d\n", global_cum_conf);
  // global death cases count
  key_idx = json.indexOf("cum_death", val_end_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf("}", val_start_idx);
  int global_cum_death = json.substring(val_start_idx, val_end_idx).toInt();
  Serial.printf("global_cum_death: %d\n", global_cum_death);

  json = getHttpsReturnStr(who_adm0_url, arcgis_root_ca);
  // Serial.printf("return: %s\n", json.c_str());

  // adm0 confirmed cases count
  key_idx = json.indexOf("features");
  key_idx = json.indexOf("cum_conf", key_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf(",", val_start_idx);
  int adm0_cum_conf = json.substring(val_start_idx, val_end_idx).toInt();
  Serial.printf("adm0_cum_conf: %d\n", adm0_cum_conf);
  // adm0 death cases count
  key_idx = json.indexOf("cum_death", val_end_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf("}", val_start_idx);
  int adm0_cum_death = json.substring(val_start_idx, val_end_idx).toInt();
  Serial.printf("adm0_cum_death: %d\n", adm0_cum_death);

  json = getHttpsReturnStr(who_adm0_new_conf_url, arcgis_root_ca);
  // Serial.printf("return: %s\n", json.c_str());

  // adm0 new confirmed cases count
  key_idx = json.indexOf("features");
  key_idx = json.indexOf("NewCase", key_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf(",", val_start_idx);
  int adm0_new_conf = json.substring(val_start_idx, val_end_idx).toInt();
  Serial.printf("adm0_new_conf: %d\n", adm0_new_conf);

  json = getHttpsReturnStr(who_adm1_url, arcgis_root_ca);
  // Serial.printf("return: %s\n", json.c_str());

  // adm1 new confirmed cases count
  key_idx = json.indexOf("features");
  key_idx = json.indexOf("new_conf", key_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf(",", val_start_idx);
  int adm1_new_conf = json.substring(val_start_idx, val_end_idx).toInt();
  Serial.printf("adm1_new_conf: %d\n", adm1_new_conf);
  // adm1 confirmed cases count
  key_idx = json.indexOf("cum_conf", key_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf(",", val_start_idx);
  int adm1_cum_conf = json.substring(val_start_idx, val_end_idx).toInt();
  Serial.printf("adm1_cum_conf: %d\n", adm1_cum_conf);
  // adm0 death cases count
  key_idx = json.indexOf("cum_death", val_end_idx);
  val_start_idx = json.indexOf(':', key_idx) + 1;
  val_end_idx = json.indexOf("}", val_start_idx);
  int adm1_cum_death = json.substring(val_start_idx, val_end_idx).toInt();
  Serial.printf("adm1_cum_death: %d\n", adm1_cum_death);

  // print data
  gfx->setFont(f3);
  y_offset = 4 + fh1 + fh2 + 4 + ((ph - fo3) / 2) + fo3;
  gfx->setTextColor(WHITE, panel_color_01);
  print_middle(4, y_offset, fw3, 0, global_cum_conf);
  gfx->setTextColor(WHITE, panel_color_02);
  print_middle(8 + pw, y_offset, fw3, 0, global_cum_death);
  y_offset += fh2;
  y_offset += ph;
  y_offset += 2;
  gfx->setTextColor(WHITE, panel_color_03);
  print_middle(4, y_offset, fw3, 0, adm0_cum_conf);
  gfx->setTextColor(WHITE, panel_color_04);
  print_middle(8 + pw, y_offset, fw3, 0, adm0_cum_death);
  y_offset += fh2;
  y_offset += ph;
  y_offset += 2;
  gfx->setTextColor(WHITE, panel_color_05);
  print_middle(4, y_offset, fw3, 0, adm1_cum_conf);
  gfx->setTextColor(WHITE, panel_color_06);
  print_middle(8 + pw, y_offset, fw3, 0, adm1_cum_death);

  gfx->setFont(f4);
  y_offset = 4 + fh1 + ((fh2 + ph + 2) * 2) - (ph / 5) + fo4;
  gfx->setTextColor(WHITE, panel_color_03);
  print_middle(4, y_offset, fw4, '+', adm0_new_conf);
  y_offset += fh2;
  y_offset += ph;
  y_offset += 2;
  gfx->setTextColor(WHITE, panel_color_05);
  print_middle(4, y_offset, fw4, '+', adm1_new_conf);

  return true;
}

void setup()
{
#if defined(ESP32)
  disableCore0WDT();
  disableCore1WDT();
  Serial.begin(115200);
#else // ESP8266
  Serial.begin(74880);
#endif
  Serial.println("Novel Coronavirus (COVID-19) Situation WHO Dashboard");

  // Connect WiFi
  Serial.println("Connection WiFi");
  WiFi.begin(SSID_NAME, SSID_PASSWORD);

  // Initialize display
  gfx->begin();
  gfx->fillScreen(BLACK);

  w = gfx->width();
  h = gfx->height();
  if (w > 319)
  {
    f1 = &FreeSans18pt7b;
    f2 = &FreeSerifBold12pt7b;
    f3 = &FreeMonoBold18pt7b;
    f4 = &FreeMonoBold9pt7b;
  }
  else if (w > 239)
  {
    f1 = &FreeSans12pt7b;
    f2 = &FreeSerifBold9pt7b;
    f3 = &FreeMonoBold12pt7b;
    f4 = 0;
  }
  else
  {
    f1 = 0;
    f2 = 0;
    f3 = 0;
    f4 = 0;
  }

  if (f1)
  {
    fh1 = f1->yAdvance;
    fo1 = fh1 * 2 / 3;
  }
  else
  {
    fh1 = 8;
    fo1 = 0;
  }
  if (f2)
  {
    fw2 = f2->glyph->xAdvance;
    fh2 = f2->yAdvance;
    fo2 = fh2 * 2 / 3;
  }
  else
  {
    fw2 = 6;
    fh2 = 8;
    fo2 = 0;
  }
  if (f4)
  {
    fw3 = f3->glyph->xAdvance;
    fh3 = f3->yAdvance;
    fo3 = fh3 * 2 / 3;
  }
  else
  {
    fw3 = 6;
    fh3 = 8;
    fo3 = 0;
  }
  if (f4)
  {
    fw4 = f4->glyph->xAdvance;
    fh4 = f4->yAdvance;
    fo4 = fh4 * 2 / 3;
  }
  else
  {
    fw4 = 6;
    fh4 = 8;
    fo4 = 0;
  }
  pw = (w - 12) / 2;
  ph = (h - 16 - fh1 - (fh2 * 3)) / 3;

  // Initialize panel colors
  banner_color = gfx->color565(0, 127, 191);
  font_color_01 = gfx->color565(255, 63, 255);
  font_color_02 = gfx->color565(63, 255, 255);
  font_color_03 = gfx->color565(255, 255, 63);
  panel_color_01 = gfx->color565(255, 0, 0);
  panel_color_02 = gfx->color565(31, 31, 31);
  panel_color_03 = gfx->color565(239, 0, 31);
  panel_color_04 = gfx->color565(31, 31, 47);
  panel_color_05 = gfx->color565(223, 0, 47);
  panel_color_06 = gfx->color565(31, 31, 63);

  // print banner
  gfx->fillRect(0, 0, w, 4 + fh1, banner_color);
  gfx->setTextColor(WHITE, banner_color);
  gfx->setFont(f1);
  y_offset = 4 + fo1;
  gfx->setCursor(0, y_offset);
  gfx->println(" COVID-19 Situation");

  gfx->setFont(f2);
  y_offset = 4 + fh1 + 3 + fo2;
  gfx->setTextColor(font_color_01, BLACK);
  gfx->setCursor(4, y_offset);
  gfx->print("Global");
  y_offset += fh2;
  y_offset += ph;
  y_offset += 4;
  gfx->setTextColor(font_color_02, BLACK);
  gfx->setCursor(4, y_offset);
  gfx->print("China");
  y_offset += fh2;
  y_offset += ph;
  y_offset += 4;
  gfx->setTextColor(font_color_03, BLACK);
  gfx->setCursor(4, y_offset);
  gfx->print("Hong Kong SAR");

  y_offset = 4 + fh1 + 4 + fh2;
  gfx->fillRoundRect(4, y_offset, pw, ph, 8, panel_color_01);
  gfx->fillRoundRect(8 + pw, y_offset, pw, ph, 8, panel_color_02);
  y_offset += fh2;
  y_offset += ph;
  y_offset += 4;
  gfx->fillRoundRect(4, y_offset, pw, ph, 8, panel_color_03);
  gfx->fillRoundRect(8 + pw, y_offset, pw, ph, 8, panel_color_04);
  y_offset += fh2;
  y_offset += ph;
  y_offset += 4;
  gfx->fillRoundRect(4, y_offset, pw, ph, 8, panel_color_05);
  gfx->fillRoundRect(8 + pw, y_offset, pw, ph, 8, panel_color_06);

  gfx->setFont(f4);
  y_offset = 4 + fh1 + 4 + fh2 + 4 + fo4;
  gfx->setTextColor(LIGHTGREY, panel_color_01);
  gfx->setCursor(10, y_offset);
  gfx->print("Confirm");
  gfx->setTextColor(LIGHTGREY, panel_color_02);
  gfx->setCursor(14 + pw, y_offset);
  gfx->print("Death");
  y_offset += fh2;
  y_offset += ph;
  y_offset += 4;
  gfx->setTextColor(LIGHTGREY, panel_color_03);
  gfx->setCursor(10, y_offset);
  gfx->print("Confirm");
  gfx->setTextColor(LIGHTGREY, panel_color_04);
  gfx->setCursor(14 + pw, y_offset);
  gfx->print("Death");
  y_offset += fh2;
  y_offset += ph;
  y_offset += 4;
  gfx->setTextColor(LIGHTGREY, panel_color_05);
  gfx->setCursor(10, y_offset);
  gfx->print("Confirm");
  gfx->setTextColor(LIGHTGREY, panel_color_06);
  gfx->setCursor(14 + pw, y_offset);
  gfx->print("Death");

  // Turn on display backlight after all drawings ready
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
}

void loop()
{
  if ((!last_rss_update_ms) || (millis() - last_rss_update_ms) > UPDATE_INTERVAL)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      updateFigures();
    }
    else
    {
      // reconnect WiFi
      WiFi.begin(SSID_NAME, SSID_PASSWORD);
      int wait_count = 0;
      while ((WiFi.status() != WL_CONNECTED) && (wait_count < 10))
      {
        delay(500);
        wait_count++;
      }
      if (WiFi.status() == WL_CONNECTED)
      {
        updateFigures();
      }
    }

    last_rss_update_ms = millis();
  }

  // let system do background task
  yield();

  // sleep a while for next update
#if defined(ESP32)
  esp_sleep_enable_timer_wakeup(UPDATE_INTERVAL * 1000L); // milliseconds to nanoseconds
  esp_light_sleep_start();
#else // ESP8266
  wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
  wifi_fpm_open();
  wifi_fpm_do_sleep(UPDATE_INTERVAL * 1000L); // milliseconds to nanoseconds
#endif
}
