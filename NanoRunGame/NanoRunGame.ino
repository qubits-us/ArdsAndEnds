#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27
#define LCD_COLUMNS 16
#define LCD_LINES   2

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);


#define COIN_SW 4
#define RELAY_PIN 3
#define BUZZ_PIN 7
#define SECS 1000
#define MINS 60 * SECS

const byte numCoinsReq = 2;
const unsigned long runTime = 5UL * MINS;
const unsigned long warningTime = 10UL * SECS;
unsigned long purchasedTime = 0;
byte coinsEntered;
unsigned long timeStart = 0;
bool running = false;
bool warned = false;

unsigned long lastDebounce = 0;
int intervalDebounce = 50;
byte lastState = 1;

byte displayCoins = 0;
int  displaySecs = 0;

unsigned long lastBuzz = 0;
int intervalBuzz = 1000;
byte buzzState = 0;


void setup() {

  pinMode(COIN_SW, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Coins: ");
  lcd.setCursor(10, 0);
  lcd.print("2");
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.setCursor(10, 1);
  lcd.print("00:00");

}

void loop() {
  //check coin counter..
  if (millis() - lastDebounce >= intervalDebounce) {
    byte state = digitalRead(COIN_SW);
    if (state != lastState) {
      lastState = state;
      lastDebounce = millis();
      if (state == HIGH) {
        coinsEntered++;
      }
    }
  }
  //do we have enough coins
  if (coinsEntered == numCoinsReq) {
    purchasedTime += runTime;
    if (!running) {
      running = true;
      warned = false;
      timeStart = millis();
      digitalWrite(RELAY_PIN, HIGH);
    }
    coinsEntered = 0;
  }
  // update display only once per change..
  if (coinsEntered != displayCoins) {
    displayCoins = coinsEntered;
    lcd.setCursor(10, 0);
    lcd.print(numCoinsReq - displayCoins);
  }

  if (running) {
    unsigned long timeRemain = purchasedTime - (millis() - timeStart);
    if (timeRemain <= warningTime && !warned) {
      //warning time..
      buzzWarning();
    }
    if (timeRemain == 0) {
      running = false;
      digitalWrite(RELAY_PIN, LOW);
      purchasedTime = 0;
    } else {
      //display time remaining..
      displayTimeRemain(timeRemain);
    }
  }
}

//display the time remaining..
void displayTimeRemain(unsigned long tr) {
  unsigned long secsLeft = tr / SECS;
  int minsLeft = 0;
  if (secsLeft > 60) {
    minsLeft = secsLeft / 60;
    secsLeft -= (minsLeft * 60);
  }
  if (secsLeft != displaySecs) {
    displaySecs = secsLeft;
    lcd.setCursor(10, 1);
    if (minsLeft < 10)
      lcd.print("0");
    lcd.print(minsLeft);
    lcd.print(":");
    if (secsLeft < 10)
      lcd.print("0");
    lcd.print(secsLeft);
  }
}

void buzzWarning() {

  switch (buzzState) {
    case 0:  tone(BUZZ_PIN, 392); lastBuzz = millis(); buzzState++; break;
    case 1: if (millis() - lastBuzz >= intervalBuzz) {
        buzzState++;
        noTone(BUZZ_PIN);
        intervalBuzz = 500;
        lastBuzz = millis();
      }
      break;
    case 2: if (millis() - lastBuzz >= intervalBuzz) {
        buzzState++;
        intervalBuzz = 1000;
        tone(BUZZ_PIN, 175);
        lastBuzz = millis();
      }
      break;
    case 3: if (millis() - lastBuzz >= intervalBuzz) {
        buzzState = 0;
        noTone(BUZZ_PIN);
        warned = true;
      }
      break;
  }
}