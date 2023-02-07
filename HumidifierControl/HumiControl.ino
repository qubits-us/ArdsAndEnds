#include <DHT.h>
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#define I2C_ADDRESS 0x3C

#define REL_PIN 13

// Define proper RST_PIN if required.
#define RST_PIN -1

#define DHTPIN 2     
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN,DHTTYPE);


SSD1306AsciiWire oled;

int col0;

float lastT = 0;
float lastH = 0;
unsigned long lastMilli = 0;
int intervalCheck = 1000;
bool RelayActive = false;





void setup() {


pinMode(REL_PIN, OUTPUT);

dht.begin();

//init i2c screen
 Wire.begin();
 Wire.setClock(400000L);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(font5x7);
  oled.clear();
  oled.set1X(); 
  oled.println(" Humidifier Control");
  oled.println();
//save the col quicker updates later..
  col0 = oled.strWidth("Temperature : ");  
  oled.println();
  oled.println("Temperature : ");
  oled.println();
  oled.println("Humidity    : ");
  oled.println();
  oled.println("Output      :");  
  
}

void loop() {


unsigned long currentMilli = millis();

if (currentMilli-lastMilli>=intervalCheck)
{

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  lastMilli = currentMilli;//after we get a good read..

  if (temperature!=lastT || humidity!=lastH)
  {
 //   EraseDisplay(lastT,lastH);
    lastT = temperature;
    lastH = humidity;


    if (humidity<75)
    {
      digitalWrite(REL_PIN, HIGH);
      RelayActive = true;
     
    } else
    {
      digitalWrite(REL_PIN, LOW);
      RelayActive = false;
    }
   
   UpdateDisplay(temperature,humidity,RelayActive);
    
  }
  
}//end interval check

}

void UpdateDisplay(byte t, byte h, bool r)
{

 oled.clearField(col0, 3, 6);  
 oled.clearField(col0, 5, 6);  
 oled.clearField(col0, 7, 10);  
 oled.setCursor(col0, 3);
 oled.print(t);oled.print("c");
 oled.setCursor(col0, 5);
 oled.print(h);oled.print("%");
 oled.setCursor(col0,7);
 if (r) {oled.print("ON");}else {oled.print("OFF");}

}
