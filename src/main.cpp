#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>
#include <ESP8266WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;

// create variable to store mac adress
String mac;

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
}

// update temp and humidity every 5 seconds
// create variable to keep track of time
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void loop()
{
  if (millis() - lastTime > timerDelay)
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
    lastTime = millis();
  }
}
