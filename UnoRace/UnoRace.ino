#include <LiquidCrystal.h>

#define FEET_PER_MILE 5280
#define TRACK_LEN 1320
#define RESET_BTN A5
const byte RS = 8;
const byte EN = 9;
const byte D4 = 10;
const byte D5 = 11;
const byte D6 = 12;
const byte D7 = 13;
const byte BAUD_RATE = 9600;
const byte LED_Prestage = 2;
const byte LED_Stage = 3;
const byte LED_Y1 = 4;
const byte LED_Y2 = 6;
const byte LED_Y3 = 7;
const byte LED_Start = A2;
const byte LED_RED_Light = A3;
const byte Pre_Stage_Sensor = A0;
const byte Stage_Sensor = A1;
const byte Finish_Sensor = A4;
const byte Start_Button = 5;

//define missing slash char for Flip ani..
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

byte flip = 0;
unsigned long AniMilli = 0;
int IntervalAni = 50;

int state = 0;
bool StartFlag = false;
unsigned long countdownStart;
unsigned long raceStart;
bool FinishFlag;
unsigned long FinishET;




LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void setup() {
  pinMode(LED_Prestage, OUTPUT);
  pinMode(LED_Stage, OUTPUT);
  pinMode(LED_Y1, OUTPUT);
  pinMode(LED_Y2, OUTPUT);
  pinMode(LED_Y3, OUTPUT);
  pinMode(LED_Start, OUTPUT);
  pinMode(LED_RED_Light, OUTPUT);
  /*
    pinMode(Pre_Stage_Sensor, INPUT);
    pinMode(Stage_Sensor, INPUT);
    pinMode(Finish_Sensor, INPUT);*/
  pinMode(Start_Button, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);
  //Serial.begin(BAUD_RATE);
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.createChar(1, slash);
  lcd.setCursor(0, 1);
  lcd.print(F("System Ready"));
  delay(2000);
  lcd.clear();
}

void loop()
{
  int Pre_Stage_Sensor_Value = analogRead(Pre_Stage_Sensor);
  int Stage_Sensor_Value = analogRead(Stage_Sensor);
  int Finish_Sensor_Value = analogRead (Finish_Sensor);
  /*
    Serial.print("PreStage: ");
    Serial.println(Pre_Stage_Sensor_Value);
    Serial.print("Stage: ");
    Serial.println(Stage_Sensor_Value);
    Serial.print("Finish: ");
    Serial.println(Finish_Sensor_Value);
    Serial.print("State: ");
    Serial.println(state);
    Serial.println();
    delay(2000);
  */
  switch (state) {
    case 0: // Standby State
      if (Pre_Stage_Sensor_Value > 500) {
        digitalWrite(LED_Prestage, LOW);
      }
      else {
        digitalWrite(LED_Prestage, HIGH);
        state++;//move to next state??
      }
      break;

    case 1:// Vehicle Staging State
      if (Stage_Sensor_Value < 500) {
        digitalWrite(LED_Stage, HIGH);
      }
      else {
        digitalWrite(LED_Stage, LOW);

      }


      if (Stage_Sensor_Value < 500) {
        lcd.clear();
        lcd.print("Vehicle Ready");
        delay(500);
        state++;
      }
      else {
        lcd.clear();
        lcd.print("Please Stage");
        delay(500);

      }
      break;

    case 2:
      if (Stage_Sensor_Value > 501) {
        state--;
      }
      else
      {

        if (digitalRead(Start_Button) == LOW)
        {
          countdownStart = millis();
          state++;
        }
      }
      break;

    case 3:

      if (millis() - countdownStart > 2000)
      {  //check sensor just before dropping the flag
        if (Stage_Sensor_Value < 500){        
        digitalWrite(LED_Y3, LOW);
        digitalWrite(LED_Start, HIGH);
        StartFlag = true;
        state++;} else{
             digitalWrite(LED_RED_Light, HIGH);
             delay(1000);
             digitalWrite(LED_RED_Light, LOW);
             lcd.clear();
             lcd.print("*!!Bad Start!!*");
             state= 7;
        }


      }
      else if (millis() - countdownStart > 1500)
      {
        digitalWrite(LED_Y2, LOW);
        digitalWrite(LED_Y3, HIGH);
      }
      else if (millis() - countdownStart > 1000)
      {
        digitalWrite(LED_Y1, LOW);
        digitalWrite(LED_Y2, HIGH);
      }
      else if (millis() - countdownStart > 500)
      {
        digitalWrite(LED_Y1, HIGH);
      }
      break;

    case 4: //check for false start??
      if (analogRead(Stage_Sensor) > 500 && !StartFlag)
      {
        digitalWrite(LED_RED_Light, HIGH);
        lcd.clear();
        lcd.print("*!!Bad Start!!*");
        state = 7;
      } else state++;

      break;
    case 5:

      if ((millis() - countdownStart > 2000))
      {

        lcd.clear();
        lcd.print("   GO ");
        //raceStart = millis() - countdownStart;??
        raceStart = millis();
        lcd.print(raceStart);
        state++;
      }
      break;
    case 6:
      if (analogRead(Finish_Sensor) < 500)
      { if (!FinishFlag)
        { FinishFlag = true;
          FinishET = millis() - raceStart;
          lcd.clear();
          lcd.print("Finish ET:");
          float secs = float(FinishET) / 1000;
          lcd.print(secs);
          lcd.setCursor(0, 1);
          lcd.print("mph:");

          float fps = (TRACK_LEN / secs);
          Serial.print("fps:"); Serial.println(fps, 4);
          float mph = (fps / FEET_PER_MILE) / 0.00028;
          Serial.print("mph:"); Serial.println(mph, 4);
          lcd.print(mph, 4);
          state = 7;
        }
      } else FlipAni();
      break;
    case 7:
      if (digitalRead(RESET_BTN) == LOW)
      {
        digitalWrite(LED_Start, LOW);
        StartFlag = false;
        FinishFlag = false;
        state = 0;
      }
      break;

  }
}// END LOOP BRACKETS

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