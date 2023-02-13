#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <ezButton.h>

#define I2C_ADDRESS 0x3C

#define REL_PIN 13

// Define proper RST_PIN if required.
#define RST_PIN -1
SSD1306AsciiWire lcd;

int upButton = 22;
int downButton = 24;
int selectButton = 26;


ezButton buttonUp(upButton);
ezButton buttonDwn(downButton);
ezButton buttonSel(selectButton);
int lastMenu = 0;
int CurrentMenu = 0;
unsigned long buValue = 0; // To store value of the up button
unsigned long bdValue = 0; // To store value of the down button
unsigned long bsValue = 0; // To store value of the select button

void setup() {
  // put your setup code here, to run once:
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  buttonUp.setDebounceTime(50);//50 ms debounce
  buttonUp.setCountMode(COUNT_RISING);
  buttonDwn.setDebounceTime(50);//50 ms debounce
  buttonDwn.setCountMode(COUNT_RISING);
  buttonSel.setDebounceTime(50);//50 ms debounce
  buttonSel.setCountMode(COUNT_RISING);

  //init i2c screen
  Wire.begin();
  Wire.setClock(400000L);

  lcd.begin(&Adafruit128x64, I2C_ADDRESS);
  lcd.setFont(font5x7);
  lcd.clear();
  lcd.set1X();
  lcd.println(" Menu System");
  lcd.println();
  lcd.println("Main Menu ");

}

void loop() {
  // put your main code here, to run repeatedly:

  menuPump();



}

void menuPump()
{
  buttonUp.loop(); // MUST call the loop() function first
  buttonDwn.loop(); // MUST call the loop() function first
  buttonSel.loop(); // MUST call the loop() function first

  unsigned long count = buttonUp.getCount();

  if (count != buValue)
  {
    buValue = count;
    //going up
    CurrentMenu--;
    if (CurrentMenu < 0)CurrentMenu = 0;
  }

  count = buttonDwn.getCount();

  if (count != bdValue)
  {
    bdValue = count;
    //Going down
    CurrentMenu++;
    if (CurrentMenu > 2)CurrentMenu = 2;
  }

  count = buttonSel.getCount();

  if (count != bsValue)
  {
    bsValue = count;
    //Select
    switch (CurrentMenu)
    {
      case 0: //first func
        break;
      case 1: //func 2
        break;
      case 2: //func 3
        break;
    }
  }

  if (lastMenu != CurrentMenu)
  {
    lastMenu = CurrentMenu;

    //top menus??
    switch (CurrentMenu)
    {
      case 0: menuHome(); break;
      case 1: menuFirst(); break;
      case 2: menuSecond(); break;

    }
  }

}


void menuHome() {
  lcd.clear();
  lcd.print("Home Screen");
  lcd.setCursor(0, 1);
  lcd.print("Use Btns to NAV");
}


void menuFirst() {
  lcd.clear();
  lcd.print("     First    ");
  lcd.setCursor(0, 1);
  lcd.print("First Top Menu");

}


void menuSecond() {
  lcd.clear();
  lcd.print("    Second");
  lcd.setCursor(0, 1);
  lcd.print("2nd Top Menu");
}


