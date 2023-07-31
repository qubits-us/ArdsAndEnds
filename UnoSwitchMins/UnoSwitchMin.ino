#define RST_BTN 2
int mins = 0;
int lastMins = 0;
unsigned long offset = 0;

unsigned long lastDebounce = 0;
int intervalDebounce = 50;
byte lastBtn = 1;

void setup() {
  Serial.begin(115200);
  pinMode(RST_BTN, INPUT_PULLUP);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(3, LOW); //led 3 used as indicator for start of time sequence

}


void loop() {

  mins = (millis() - offset) / 60000UL;
  if (mins != lastMins) {
    lastMins = mins;
    Serial.println(mins);

    switch (mins) {
      case 1: digitalWrite(4, LOW); break;
      case 2: digitalWrite(5, LOW); break;
      case 3: digitalWrite(6, LOW); break;
      case 4: digitalWrite(7, LOW); digitalWrite(3, HIGH); break;
    }
  }

  //reset button code to restart sequence..
  if (millis() - lastDebounce >= intervalDebounce) {
    byte btn = digitalRead(RST_BTN);
    if (btn != lastBtn) {
      lastDebounce = millis();
      lastBtn = btn;
      if (btn == LOW) {
        digitalWrite(4, HIGH);
        digitalWrite(5, HIGH);
        digitalWrite(6, HIGH);
        digitalWrite(7, HIGH);
        digitalWrite(3, LOW);
        offset = millis();
        mins = 0;
        lastMins = 0;
      }
    }
  }

}