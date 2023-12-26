#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60

char mac[18]; // Change to char array

uint8_t broadcastAddress[] = {0xE0, 0x5A, 0x1B, 0xA1, 0x9B, 0x00};
typedef struct struct_message
{
  char a[32];
  float b;
  float c;
} struct_message;
struct_message myData;

bool deviceConnected = false;

float t = 0.0;
float h = 0.0;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  // Serial.print("Last Packet Send Status: ");
  // if (sendStatus == 0)
  // {
  //   Serial.println("Delivery success");
  //   deviceConnected = true;
  // }
  // else
  // {
  //   Serial.println("Delivery fail");
  //   deviceConnected = false;
  // }
}

void setup()
{
  dht.begin();

  // Use char array instead of String
  WiFi.macAddress().toCharArray(mac, 18);
  // Serial.print("ESP8266 Board MAC Address:  ");
  // Serial.println(mac);

  Wire.begin(); // Initialize the I2C library

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0)
  {
    // Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop()
{
  t = dht.readTemperature();
  h = dht.readHumidity();

  strcpy(myData.a, "DHT22");
  myData.b = t;
  myData.c = h;

  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  // Serial.println("Going to sleep now...");
  ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR, RF_CAL);
  delay(100); // Some delay to allow ESP8266 to enter deep sleep properly
}
