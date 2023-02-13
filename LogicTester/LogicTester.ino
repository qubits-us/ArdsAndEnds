
/*
   Logic Tester..
   Encoder menu..
   lcd 16x2
   created 2/12/2023 ~q

*/

#include <LiquidCrystal_I2C.h>
#include <ezButton.h>
//screen
LiquidCrystal_I2C lcd(0x27, 16, 2);
//encoder
#define BTN_UP   13
#define BTN_DOWN 12
#define BTN_SEL  11
//test pins for logic gate
#define pinINA 2
#define pinINB 3
#define pinOUT 4

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

int xValue = 0; // To store value of the X axis
int xLast =  0;//store the previous value of x y
int CurrentMenu = 0;
int CurrentSub = 0;
unsigned long bValue = 0;//counter for button presses

//timer and interval for encoder debouncing
unsigned long lastSample = 0;
int intervalSample = 50;
//select button on encoder..
ezButton button(BTN_SEL);
//answer from yes/no prompt..
bool Proceed = true;
//forward decs..
bool gateTestXNOR(byte loops = 1);
bool gateTestAND(byte loops = 1);
bool gateTestOR(byte loops = 1);
bool gateTestXOR(byte loops = 1);
bool gateTestNAND(byte loops = 1);
bool gateTestNOR(byte loops = 1);
//vars for serial input
char ChipToTest[10];
int  CurrChar = 0;
bool CommandRecvd = false;
//for fancy screen progress
int TestProgress = 0;
int TestSteps = 2000;




void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, slash);
  lcd.setCursor(1, 0);
  lcd.print("Starting Up..");
  Serial.println("Starting Up..");
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  button.setDebounceTime(50);//50 ms debounce
  button.setCountMode(COUNT_RISING);
  menuHome();
  PromptChip();
}

int lastClk = HIGH;

void loop() {
  menuPump();

  if (Serial.available())
  {
    while (Serial.available())
    {
      char aChar = Serial.read();
      if (aChar != 10 && aChar != 13)
      {
        if (CurrChar < 10)ChipToTest[CurrChar] = aChar;
        CurrChar++;
      } else {
        CommandRecvd = true;
      }
    }

  }

  if (CommandRecvd)
  {
    int chip = -1;
    if (ChipToTest[0] == '1')chip = 0;
    if (ChipToTest[0] == '2')chip = 1;
    if (ChipToTest[0] == '3')chip = 2;
    if (ChipToTest[0] == '4')chip = 3;
    if (ChipToTest[0] == '5')chip = 4;
    if (ChipToTest[0] == '6')chip = 5;
    CommandRecvd = false;
    CurrChar = 0;
    switch (chip) {
      case 0: if (gateTestAND(2)) {
          Serial.println("Chip Passed.");
          scrnTestPass();

        }
        else
          Serial.println("Chip failed!");
        goHome();
        break;
      case 1: if (gateTestOR(1)) {
          Serial.println("Chip Passed.");
          scrnTestPass();
        }
        else
          Serial.println("Chip failed!");
        goHome();
        break;
      case 2: if (gateTestXOR(1)) {
          Serial.println("Chip Passed.");
          scrnTestPass();
        }
        else
          Serial.println("Chip failed!");
        goHome();
        break;
      case 3: if (gateTestXOR(1)) {
          Serial.println("Chip Passed.");
          scrnTestPass();
        }
        else
          Serial.println("Chip failed!");
        goHome();
        break;
      case 4: if (gateTestNAND(1)) {
          Serial.println("Chip Passed.");
          scrnTestPass();
        }
        else
          Serial.println("Chip failed!");
        goHome();
        break;
      case 5: if (gateTestNOR(1)) {
          Serial.println("Chip Passed.");
          scrnTestPass();
        }
        else
          Serial.println("Chip failed!");
        goHome();
        break;
      case 6: if (gateTestXNOR(1)) {
          Serial.println("Chip Passed.");
          scrnTestPass();
        }
        else
          Serial.println("Chip failed!");
        goHome();
        break;
    }

    PromptChip();
  }
}

void PromptChip()
{
  Serial.println("logic gate tester v0.1");
  delay(1000);
  Serial.println("What logic gate would you like to test today?");
  Serial.println("<1=AND><2=<OR><3=XOR><4=NAND><5=NOR><6=XNOR>");


}


void menuPump()
{
  button.loop(); // MUST call the loop() function first
  bool gotClick = false;

  unsigned long currMillis = millis();
  if (currMillis - lastSample >= intervalSample)
  {

    unsigned long count =  button.getCount();

    if (count != bValue)
    {
      bValue = count;
      if (CurrentSub == 0)
      {
        CurrentSub = CurrentMenu;
        menuConfirmYes();


      } else {
        switch (CurrentSub) {
          case 1: //sub 1
            if (Proceed) {
              if (gateTestAND(1)) scrnTestPass(); else scrnTestFail();
              goHome();
            } else {
              Serial.println("Test aborted");
              goHome();

            }
            break;
          case 2: //sub 2
            if (Proceed) {
              if (gateTestOR(1)) scrnTestPass(); else scrnTestFail();
              goHome();
            } else {
              Serial.println("Test aborted");
              goHome();
            }
            break;
          case 3: //sub 2
            if (Proceed) {
              if (gateTestXOR(1)) scrnTestPass(); else scrnTestFail();
              goHome();
            } else {
              Serial.println("Test aborted");
              goHome();
            }
            break;
          case 4: //sub 2
            if (Proceed) {
              if (gateTestNAND(1)) scrnTestPass(); else scrnTestFail();
              goHome();
            } else {
              Serial.println("Test aborted");
              goHome();
            }
            break;
          case 5: //sub 2
            if (Proceed) {
              if (gateTestNOR(1)) scrnTestPass(); else scrnTestFail();
              goHome();
            } else {
              Serial.println("Test aborted");
              goHome();
            }
            break;
          case 6: //sub 2
            if (Proceed) {
              if (gateTestXNOR(1)) scrnTestPass(); else scrnTestFail();
              goHome();
            } else {
              Serial.println("Test aborted");
              goHome();
            }
            break;
        }
      }
    }



    //encoder movement tracking..
    int newClk = digitalRead(BTN_UP);
    if (newClk != lastClk) {
      // There was a change on the CLK pin
      lastClk = newClk;

      int dtValue = digitalRead(BTN_DOWN);
      if (newClk == LOW && dtValue == HIGH) {
        xValue = 1024;
        gotClick = true;
      }
      if (newClk == LOW && dtValue == LOW) {
        xValue = 0;
        gotClick = true;
      }
    }
  }




  int lastMenu = CurrentMenu;
  int lastSub  = CurrentSub;
  //ignore up/down if in a sub menu
  if (gotClick && CurrentSub == 0)
  {
    if (xValue == 0)
    {
      //down
      if (CurrentMenu < 6)CurrentMenu++;

    } else if (xValue > 1000)
    {
      //up
      if (CurrentMenu > 0)CurrentMenu--;

    }
  }

  if (gotClick && CurrentSub != 0 && CurrentMenu != 0)
  {
    Proceed = !Proceed;
    if (Proceed) {
      menuConfirmYes();
    } else {
      menuConfirmNo();
    };

  }


  // only switch menu once..
  if (lastMenu != CurrentMenu)
  {
    menuHome();
  }

}


void goHome()
{
  CurrentSub = 0;
  CurrentMenu = 0;
  Proceed = true;
  menuHome();
}

void scrnTestPass() {
  lcd.clear();
  lcd.print("Logic Gate Test");
  lcd.setCursor(0, 1);
  lcd.print("    PASSED");
  delay(2000);
}

void scrnTestFail() {
  lcd.clear();
  lcd.print("Logic Gate Test");
  lcd.setCursor(0, 1);
  lcd.print("     FAILED");
  delay(2000);
}

void scrnTestBegin() {
  lcd.clear();
  switch (CurrentSub) {
    case 1: lcd.print("X  Testing AND X"); break;
    case 2: lcd.print("X  Testing OR  X"); break;
    case 3: lcd.print("X  Testing XOR X"); break;
    case 4: lcd.print("X Testing NAND X"); break;
    case 5: lcd.print("X  Testing NOR X"); break;
    case 6: lcd.print("X Testing XNOR X"); break;
  }
  lcd.setCursor(0, 1);
  lcd.print("");
  delay(100);
}



void scrnTestWork(int workMillis = 500)
{
  int updateInterval = 0;
  byte flip = 0;

  for (int i = 0; i < workMillis; i++)
  {
    TestProgress++;

    updateInterval ++;
    if (updateInterval > 10)
    {
      if (TestProgress < TestSteps)
      {
        int digi = TestProgress / (TestSteps / 16);
        lcd.setCursor(digi, 1);
        lcd.print(char(255));
      }

      //update screen
      updateInterval = 0;//reset counter

      switch (flip) {
        case 0: flip = 1; lcd.setCursor(0, 0); lcd.print("|"); lcd.setCursor(15, 0); lcd.print("|"); break;
        case 1: flip = 2; lcd.setCursor(0, 0); lcd.print("/"); lcd.setCursor(15, 0); lcd.print("/"); break;
        case 2: flip = 3; lcd.setCursor(0, 0); lcd.print("-"); lcd.setCursor(15, 0); lcd.print("-"); break;
        case 3: flip = 4; lcd.setCursor(0, 0); lcd.print("\x01"); lcd.setCursor(15, 0); lcd.print("\x01"); break;
        case 4: flip = 0; lcd.setCursor(0, 0); lcd.print("|"); lcd.setCursor(15, 0); lcd.print("|"); break;
      }

    }
    delay(1);
  }


}



void menuHome() {
  lcd.clear();
  lcd.print("Logic Gate Test");
  lcd.setCursor(0, 1);
  switch (CurrentMenu)
  {
    case 0: lcd.print("Spin to NAV"); break;
    case 1: lcd.print("AND gate"); break;
    case 2: lcd.print("OR gate"); break;
    case 3: lcd.print("XOR gate"); break;
    case 4: lcd.print("NAND gate"); break;
    case 5: lcd.print("NOR gate"); break;
    case 6: lcd.print("XNOR gate"); break;
  }

}




void menuConfirmYes() {
  lcd.clear();
  lcd.print("Shall we begin?");
  lcd.setCursor(0, 1);
  lcd.print("[Yes]  No");

}
void menuConfirmNo() {
  lcd.clear();
  lcd.print("Shall we begin?");
  lcd.setCursor(0, 1);
  lcd.print("Yes  [No]");

}





bool gateTestAND(byte loops = 1)
{
  bool goodChip = true;
  scrnTestBegin();
  TestProgress = 0;
  Serial.print("Testing AND loops:");
  Serial.print(loops);
  for (int i = 0; i < loops; i++)
  {

    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    if (goodChip) {
      Serial.print(".");
    } else {
      Serial.print("x");
      break;
    }

  }
  return goodChip;
}

bool gateTestOR(byte loops = 1)
{
  bool goodChip = true;
  scrnTestBegin();
  TestProgress = 0;
  Serial.print("Testing OR loops:");
  Serial.print(loops);
  for (int i = 0; i < loops; i++)
  {

    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    if (goodChip) {
      Serial.print(".");
    } else {
      Serial.print("x");
      break;
    }

  }
  return goodChip;
}

bool gateTestXOR(byte loops = 1)
{
  bool goodChip = true;
  scrnTestBegin();
  TestProgress = 0;
  Serial.print("Testing XOR loops:");
  Serial.print(loops);
  for (int i = 0; i < loops; i++)
  {

    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    if (goodChip) {
      Serial.print(".");
    } else {
      Serial.print("x");
      break;
    }

  }
  return goodChip;
}


bool gateTestNAND(byte loops = 1)
{
  bool goodChip = true;
  scrnTestBegin();
  TestProgress = 0;
  Serial.print("Testing NAND loops:");
  Serial.print(loops);
  for (int i = 0; i < loops; i++)
  {

    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    if (goodChip) {
      Serial.print(".");
    } else {
      Serial.print("x");
      break;
    }

  }
  return goodChip;
}

bool gateTestNOR(byte loops = 1)
{
  bool goodChip = true;
  scrnTestBegin();
  TestProgress = 0;
  Serial.print("Testing NOR loops:");
  Serial.print(loops);
  for (int i = 0; i < loops; i++)
  {

    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    if (goodChip) {
      Serial.print(".");
    } else {
      Serial.print("x");
      break;
    }

  }
  return goodChip;
}


bool gateTestXNOR(byte loops = 1)
{
  bool goodChip = true;
  scrnTestBegin();
  TestProgress = 0;

  Serial.print("Testing XNOR loops:");
  Serial.print(loops);
  for (int i = 0; i < loops; i++)
  {
    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    digitalWrite(pinINA, 0);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 0);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 1) {
      goodChip = false;
    }
    digitalWrite(pinINA, 1);
    digitalWrite(pinINB, 1);
    scrnTestWork(500);
    if (digitalRead(pinOUT) == 0) {
      goodChip = false;
    }
    if (goodChip) {
      Serial.print(".");
    } else {
      Serial.print("x");
      break;
    }
  }
  return goodChip;
}
