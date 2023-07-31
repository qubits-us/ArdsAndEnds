#define DATA_PIN  8  // Pin connected to DS of 74HC595
#define LATCH_PIN 9  // Pin connected to STCP of 74HC595
#define CLOCK_PIN 10 // Pin connected to SHCP of 74HC595
#define BTN_PIN 2 //Pin for button

// How many of the shift registers
#define NUM_SHIFT_REGS 2

const uint8_t numOfRegisterPins = (NUM_SHIFT_REGS * 8) - 6;


unsigned level [] = { 0, 100, 300, 500, 700, 900, 1100, 1300, 1500, 1700 };
int Nlvl = sizeof(level) / sizeof(int);
byte butLst;
unsigned long msecBut;
unsigned long lastChange;

bool registers[numOfRegisterPins];

void setup() {
  Serial.begin(115200);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  clearRegisters();
  writeRegisters();
}


void loop() {
  unsigned long msec = millis ();

  byte but = digitalRead (BTN_PIN);
  if (butLst != but)  {
    butLst = but;
    lastChange = millis();
    delay (20);         // debounce

    if (LOW == but)     // press
      msecBut = msec;
    else {               // release
      Serial.println (getLevel (msec - msecBut));
      clearRegisters();
      writeRegisters();
    }
  } else if (but == LOW) {

    int lvl = getLevel(msec - lastChange);
    //  Serial.println(lvl);

    switch (lvl) {
      case 1: setRegisterPin(0, HIGH); break;
      case 2: setRegisterPin(1, HIGH); break;
      case 3: setRegisterPin(2, HIGH); break;
      case 4: setRegisterPin(3, HIGH); break;
      case 5: setRegisterPin(4, HIGH); break;
      case 6: setRegisterPin(5, HIGH); break;
      case 7: setRegisterPin(6, HIGH); break;
      case 8: setRegisterPin(7, HIGH); break;
      case 9: setRegisterPin(8, HIGH); break;
      case 10: setRegisterPin(9, HIGH); break;
    }

    writeRegisters();

  }
}

int getLevel (unsigned val)
{
  for (int lvl = 0; lvl < Nlvl; lvl++)
    if (level [lvl] > val)
      return lvl;
  return Nlvl;
}

void clearRegisters() {
  // Reset all register pins
  for (int i = numOfRegisterPins - 1; i >= 0; i--) {
    registers[i] = LOW;
  }
}

void setRegisterPin(int index, int value) {
  // Set an individual pin HIGH or LOW
  registers[index] = value;
}

void writeRegisters() {
  // Set and display registers
  digitalWrite(LATCH_PIN, LOW);
  for (int i = numOfRegisterPins - 1; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, registers[i]);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);
}
