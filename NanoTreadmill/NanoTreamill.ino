#include <LiquidCrystal_I2C.h>

#define FEET_PER_MILE 5280
#define TRACK_LEN 4

#define RELAY_PIN 6
#define START_BTN 4
#define PAUSE_BTN 3
#define STOP_BTN  2
#define COUNT_PIN 5

#define PAUSED 0
#define RUNNING 1
#define STOPPED 2



//define missing slash char for spin ani..
uint8_t slash[8] = {
  0b10000,
  0b10000,
  0b01000,
  0b00100,
  0b00100,
  0b00010,
  0b00001,
  0b00001,
};


//screen
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte flip = 0;
unsigned long AniMilli = 0;
int IntervalAni = 50;


byte buttons[4] = {START_BTN, PAUSE_BTN, STOP_BTN, COUNT_PIN};
unsigned long lastDebounce[4];
byte lastStates[4];
byte buttonStates[4];
byte intervalDebounce = 50;
bool running = false;
byte state = PAUSED;
byte lastState = PAUSED;

unsigned long count, lastCount;
unsigned long time, lastTime;


void setup() {
  pinMode(START_BTN, INPUT_PULLUP);
  pinMode(PAUSE_BTN, INPUT_PULLUP);
  pinMode(STOP_BTN, INPUT_PULLUP);
  pinMode(COUNT_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, slash);
  lcd.setCursor(1, 0);
  lcd.print("MPH   : ");
  lcd.setCursor(1, 1);
  lcd.print("Miles : ");
}


void loop() {

  for (int i = 0; i < 4; i++) {
    if (millis() - lastDebounce[i] >= intervalDebounce) {
      buttonStates[i] = digitalRead(buttons[i]);
      if (buttonStates[i] != lastStates[i]) {
        lastDebounce[i] = millis();
        lastStates[i] = buttonStates[i];
        if (buttonStates[i] == LOW) {
          switch (buttons[i]) {
            case START_BTN: state = RUNNING; break;
            case PAUSE_BTN: state = PAUSED; if (!running && lastState != STOPPED) lastState = RUNNING; break;
            case STOP_BTN : state = STOPPED; break;
            case COUNT_PIN: if (running) count++; time = millis(); break;
          }
        }
      }
    }
  }

  if (state != lastState) {
    switch (state) {

      case RUNNING: if (!running) {
          running = true;
          count = 0;
          lastCount = 1;
          digitalWrite(RELAY_PIN, HIGH);
        }
        lcd.backlight();
        lcd.display();
        break;
      case PAUSED : if (running) {
          running = false;
          digitalWrite(RELAY_PIN, LOW);
        } else if (lastState != STOPPED) {
          running = true;
          state = RUNNING;
          digitalWrite(RELAY_PIN, HIGH);
        }
        lcd.backlight();
        lcd.display();
        break;
      case STOPPED: running = false;
        digitalWrite(RELAY_PIN, LOW);
        lcd.noBacklight();
        lcd.noDisplay();
        break;

    }
    lastState =  state;
  }



  if (running) {
    FlipAni();
    if (count != lastCount) {
      lastCount = count;
      unsigned long elapsed = time - lastTime;
      lastTime = time;
      float secs = float(elapsed) / 1000;
      float fps = (TRACK_LEN / secs);
      float mph = (fps / FEET_PER_MILE) / 0.00028;
      lcd.setCursor(9, 0);
      lcd.print("     ");
      lcd.setCursor(9, 0);
      lcd.print(mph, 2);
      float miles =  float((count * TRACK_LEN)) / float(FEET_PER_MILE);
      lcd.setCursor(9, 1);
      lcd.print(miles, 4);
    }
  }
}


void FlipAni() {
  if (millis() - AniMilli >= IntervalAni)
  {
    AniMilli = millis();
    switch (flip) {
      case 0: flip = 1; lcd.setCursor(0, 0); lcd.print("|"); lcd.setCursor(15, 0); lcd.print("|"); break;
      case 1: flip = 2; lcd.setCursor(0, 0); lcd.print("/"); lcd.setCursor(15, 0); lcd.print("/"); break;
      case 2: flip = 3; lcd.setCursor(0, 0); lcd.print("-"); lcd.setCursor(15, 0); lcd.print("-"); break;
      case 3: flip = 4; lcd.setCursor(0, 0); lcd.print("\x01"); lcd.setCursor(15, 0); lcd.print("\x01"); break;
      case 4: flip = 0; lcd.setCursor(0, 0); lcd.print("|"); lcd.setCursor(15, 0); lcd.print("|"); break;
    }
  }
}