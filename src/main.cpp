#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;

// create variable to store mac adress
String mac;
// REPLACE WITH RECEIVER MAC Address - 48:3F:DA:AA:0E:B9
// sender mac address: 80:7D:3A:48:CD:FD
uint8_t broadcastAddress[] = {0x48, 0x3F, 0xDA, 0xAA, 0x0E, 0xB9};
// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
  char a[32];
  float b;
  float c;
  String d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
  }
  else
  {
    Serial.println("Delivery fail");
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(0, 2);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  if (!aht.begin())
  {
    Serial.println("Could not find AHT? Check wiring");
    while (1)
      delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  mac = WiFi.macAddress();
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

// update temp and humidity every 5 seconds
// create variable to keep track of time
unsigned long sensorLastTime = 0;
unsigned long sensorTimerDelay = 5000;

void loop()
{
  if (millis() - sensorLastTime > sensorTimerDelay)
  {
    aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
    Serial.println(temp.temperature);
    Serial.println(humidity.relative_humidity);
    // display text on screen
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
    display.setCursor(10, 10);
    display.print(mac);
    display.display();
    sensorLastTime = millis();

    // Set values to send
    strcpy(myData.a, "THIS IS A SENSOR READINGS");
    myData.b = temp.temperature;
    myData.c = humidity.relative_humidity;
    myData.d = "-----------------";

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  }
}
