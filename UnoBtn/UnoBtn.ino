/*
  Single button, counting presses

  2.25.2023 ~q
*/
#define BTN_PIN 2


byte buttonPresses = 0;
unsigned long lastPress = 0;
int intervalDebounce = 50;
byte lastState = 1;// pulled up
bool timerStarted = false;
unsigned long timerStart = 0;
int intervalTimer = 5000;//5 seconds

void setup()
{
  pinMode(BTN_PIN, INPUT_PULLUP);// Set the switch pin as input with pull up resistor
  Serial.begin(9600);
  Serial.println("Ready");
}

void loop() {

  if (millis() - lastPress >= intervalDebounce) {
    lastPress = millis();
    byte state = digitalRead(BTN_PIN);
    if (state != lastState) {
      lastState = state;
      //start timer to count press..
      if (!timerStarted) {
        timerStarted = true;
        timerStart = millis();
      }
      if (state == HIGH) buttonPresses++;
    }
  }

  if (timerStarted) {
    if (millis() - timerStart >= intervalTimer) {
      Serial.print("Pressed:");
      Serial.println(buttonPresses);
      //do something with presses..


      //reset for next shot
      timerStarted = false;
      buttonPresses = 0;

    }
  }
}