#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_ADS1X15.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;

Adafruit_ADS1115 ads;

char mac[18]; // Change to char array

uint8_t broadcastAddress[] = {0x48, 0x3F, 0xDA, 0xAA, 0x0E, 0xB9};
typedef struct struct_message
{
  char a[32];
  float b;
  float c;
} struct_message;
struct_message myData;

bool deviceConnected = false;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
    deviceConnected = true;
  }
  else
  {
    Serial.println("Delivery fail");
    deviceConnected = false;
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(0, 2);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while (1)
      ; // Don't proceed, loop forever
  }
  if (!aht.begin())
  {
    Serial.println("Could not find AHT? Check wiring");
    while (1)
      delay(10);
  }
  Serial.println("AHT10 or AHT20 found");

  // Use char array instead of String
  WiFi.macAddress().toCharArray(mac, 18);
  Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(mac);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1)
      ;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

unsigned long sensorLastTime = 0;
unsigned long sensorTimerDelay = 5000;
int soilMoistureValue = 0;
int airValue = 11500;
int waterValue = 4500;

void loop()
{
  if (millis() - sensorLastTime > sensorTimerDelay)
  {
    aht.getEvent(&humidity, &temp);

    int16_t adc0, adc1, adc2, adc3;
    float volts0, volts1, volts2, volts3;

    adc0 = ads.readADC_SingleEnded(0);
    adc1 = ads.readADC_SingleEnded(1);
    adc2 = ads.readADC_SingleEnded(2);
    adc3 = ads.readADC_SingleEnded(3);

    volts0 = ads.computeVolts(adc0);
    volts1 = ads.computeVolts(adc1);
    volts2 = ads.computeVolts(adc2);
    volts3 = ads.computeVolts(adc3);

    // when volts0 is 1 then soil moisture is 100%, when volts0 is 2.15 then soil moisture is 0%. Make smooth transition between 1 and 2.15 to get soil moisture value
    soilMoistureValue = map(adc0, airValue, waterValue, 0, 100);

    Serial.println(temp.temperature);
    Serial.println(humidity.relative_humidity);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("T: ");
    display.setCursor(20, 0);
    display.print(static_cast<int>(temp.temperature));
    display.print("C");
    display.setCursor(60, 0);
    display.print("H: ");
    display.setCursor(80, 0);
    display.print(static_cast<int>(humidity.relative_humidity));
    display.print("%");
    display.setCursor(0, 10);
    display.print(mac);
    // display adc0m adc1, adc2, adc3 on the screen
    display.setCursor(0, 20);
    display.print("AIN0: ");
    display.setCursor(40, 20);
    display.print(volts0);
    display.print("V");
    display.setCursor(0, 30);
    display.print("AIN1: ");
    display.setCursor(40, 30);
    display.print(volts1);
    display.print("V");
    display.setCursor(0, 40);
    display.print("AIN2: ");
    display.setCursor(40, 40);
    display.print(volts2);
    display.print("V");
    display.setCursor(0, 50);
    display.print("AIN3: ");
    display.setCursor(40, 50);
    display.print(volts3);
    display.print("V");

    display.setCursor(80, 20);
    display.print("SM: ");
    display.setCursor(100, 20);
    display.print(soilMoistureValue);

    display.setCursor(115, 10);
    if (deviceConnected)
    {
      display.print("1");
    }
    else
    {
      display.print("0");
    }
    display.display();

    strcpy(myData.a, "THIS IS A SENSOR READINGS");
    myData.b = temp.temperature;
    myData.c = humidity.relative_humidity;

    esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

    Serial.println("-----------------------------------------------------------");
    Serial.print("AIN0: ");
    Serial.print(adc0);
    Serial.print("  ");
    Serial.print(volts0);
    Serial.println("V");
    Serial.print("AIN1: ");
    Serial.print(adc1);
    Serial.print("  ");
    Serial.print(volts1);
    Serial.println("V");
    Serial.print("AIN2: ");
    Serial.print(adc2);
    Serial.print("  ");
    Serial.print(volts2);
    Serial.println("V");
    Serial.print("AIN3: ");
    Serial.print(adc3);
    Serial.print("  ");
    Serial.print(volts3);
    Serial.println("V");

    sensorLastTime = millis();
  }
}
