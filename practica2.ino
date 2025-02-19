/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsEsp32Mqtt.h"
#include "DHT.h"
#include <TFT_eSPI.h>
#include <SPI.h>

/****************************************
 * Define Constants
 ****************************************/
#define DHTPIN 33
#define DHTTYPE DHT11

const char *UBIDOTS_TOKEN = "BBUS-yqHu5Lt4V4I9qFg87DP1wkwwS5Dafd";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "KaiaDeco";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "18292824";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";   // Replace with the device label to subscribe to
const char *VARIABLE_LABEL_SW1 = "sw1"; // Replace with your variable label to subscribe to
const char *VARIABLE_LABEL_SW2 = "sw2";
const char *VARIABLE_LABEL_1 = "temperatura"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL_2 = "humedad";

const uint8_t LED1 = 26; // Pin used to write data based on 1's and 0's coming from Ubidots
const uint8_t LED2 = 25;

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds
const int OLED_FREQUENCY = 1000;

unsigned long publish_timer;
unsigned long oled_timer;

bool sw1, sw2 = false;

Ubidots ubidots(UBIDOTS_TOKEN);
TFT_eSPI tft = TFT_eSPI();
DHT dht(DHTPIN, DHTTYPE);

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char *topic, byte *payload, unsigned int length)
{
  if (strstr(topic, "sw1") != NULL) {
    if ((char)payload[0] == '1')
    {
      sw1 = true;
    }
    else
    {
      sw1 = false;
    }   
  }
  if (strstr(topic, "sw2") != NULL) {
    if ((char)payload[0] == '1')
    {
      sw2 = true;
    }
    else
    {
      sw2 = false;
    }   
  }  
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SW1); // Insert the device and variable's Labels, respectively
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SW2);

  dht.begin();

  tft.init();
  tft.fillScreen(TFT_BLACK);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }  
  
  if (abs((int)(millis() - oled_timer)) > OLED_FREQUENCY) // triggers the routine every 5 seconds
  {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_BLUE);
    tft.drawString("Temperatura: ", tft.width() / 2 - 5 * 8, 50, 2);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(String(t) + " C", tft.width() / 2 - 3 * 8, 70, 2);
    tft.setTextColor(TFT_BLUE);
    tft.drawString("Humedad: ", tft.width() / 2 - 3 * 8, 110, 2);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(String(h) + " %", tft.width() / 2 - 3 * 8, 130, 2);
    oled_timer = millis();

    if (sw1) {
      tft.fillCircle(tft.width() / 2 - 25, 180, 15, TFT_GREEN); 
      digitalWrite(LED1, HIGH);

    } else {
      tft.fillCircle(tft.width() / 2 - 25, 180, 15, TFT_SILVER);
      digitalWrite(LED1, LOW);
    }
    if (sw2) {
      tft.fillCircle(tft.width() / 2 + 25, 180, 15, TFT_GREEN);
      digitalWrite(LED2, HIGH);
    } else {
      tft.fillCircle(tft.width() / 2 + 25, 180, 15, TFT_SILVER);
      digitalWrite(LED2, LOW);
    }
  }

  if (!ubidots.connected())
  {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SW1); // Insert the device and variable's Labels, respectively
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SW2);
  }
  if (abs((int)(millis() - publish_timer)) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    ubidots.add(VARIABLE_LABEL_1, t); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL_2, h);
    ubidots.publish(DEVICE_LABEL);
    publish_timer = millis();
  }
  ubidots.loop();
}
