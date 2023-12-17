#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int SensorPin = A0;
int soilMoistureValue = 0;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;
void setup()
{
  Serial.begin(115200);
  Wire.begin(1, 3);
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
  // set pin 2 as analog input
  // pinMode(SensorPin, INPUT);
}

void loop()
{
  aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
  Serial.print(temp.temperature);
  Serial.print(humidity.relative_humidity);
  // display text on screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.setCursor(50, 0);
  display.print(temp.temperature);
  display.setCursor(0, 20);
  display.print("Hum: ");
  display.setCursor(50, 20);
  display.println(humidity.relative_humidity);

  // display soil moisture from sensor
  soilMoistureValue = analogRead(SensorPin);
  display.setCursor(0, 40);
  display.print("Moisture: ");
  display.setCursor(60, 40);
  display.print(soilMoistureValue);

  display.display();
  delay(2000);
}