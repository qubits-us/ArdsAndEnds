#include <LiquidCrystal.h>
#include <ezButton.h>
//Joystick
#define VRX_PIN  A0 // Arduino A0 pin connected to VRX pin on TS
#define VRY_PIN  A1 // Arduino A1 pin connected to VRY pin on TS
#define SW_PIN   2  // Arduino digital pin 2 connected to SW  pin on TS
#define VCC2  5 //define pin 5 or any other digial pin here as VCC2

ezButton button(SW_PIN);

int xValue = 0; // To store value of the X axis
int yValue = 0; // To store value of the Y axis
int xLast = 0;//store the previous value of x y
int yLast = 0;//only print out when changes..
int CurrentMenu = 0;
int CurrentSub = 0;
//int sbStat = 0; //SB statu it has a bug with the xValue analogRead serial.print )
unsigned long bValue = 0; // To store value of the button
int dbgStat = 0; //Debug status(config menu) 0:off 1:on 
//char sbW [13];  //SB write status

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  Serial.begin(9600);
  pinMode(VCC2,OUTPUT);//define a digital pin as output
  digitalWrite(VCC2, HIGH);// set the above pin as HIGH so it acts as 5V
  button.setDebounceTime(50);//50 ms debounce
  button.setCountMode(COUNT_FALLING);
  lcd.begin(16, 2);
  menuHome();
}

void loop() {

  menuPump();

}


void menuPump()
{
 button.loop(); // MUST call the loop() function first

  unsigned long count = button.getCount();

  if (count!=bValue)
  {
    bValue=count;
    //currently only menuSub1Second handles button
    if (CurrentMenu==2 && CurrentSub==1 )
    {
      if (dbgStat>0)dbgStat=0;else dbgStat=1;
      menuSub1Second();//redraw submenu, show the changed var
    }
  }

  // read analog X and Y analog values
  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);
  
  int lastMenu = CurrentMenu;
  int lastSub  = CurrentSub;
   //ignore up/down if in a sub menu
  if (xValue!=xLast && CurrentSub==0)
  {
     if (xValue==0)
     {
       //down
       if (CurrentMenu<2)CurrentMenu++;

     } else if (xValue>1000)
     {
       //up
       if (CurrentMenu>0)CurrentMenu--;

     }
  }
  // only switch menu once..
   if (lastMenu!=CurrentMenu)
   {
     
    //top menus??
    switch (CurrentMenu)
    {
       case 0:menuHome();break;
       case 1:menuFirst();break;
       case 2:menuSecond();break;

    }
   }

    if (yLast!=yValue)
    {

        if (yValue==0)
          {
          //left
           if (CurrentSub<2)CurrentSub++;

             } else if (yValue>1000)
               {
                  //right
                  if (CurrentSub>0)CurrentSub--;

                }      

           if (CurrentSub<0) CurrentSub=0;//no negs allowed

     //sub menu movement
       if (CurrentMenu==1)
          { //menu 1 
           if (CurrentSub>1)CurrentSub=1; //only has 1 sub..
           if (CurrentSub==1){
            menuSub1First();} else if (CurrentSub==0) menuFirst();
            } else
            if (CurrentMenu==2)
            { //menu 2 
             if (CurrentSub>2)CurrentSub=2; //2 subs here
             if (CurrentSub==1){
              menuSub1Second();} else if (CurrentSub==2) menuSub2Second();else
                    if (CurrentSub == 0) menuSecond();
            }
          
     }

  // see if we need to print..
  if (xValue!=xLast || yValue!=yLast)
  {
    xLast=xValue;
    yLast=yValue;
 if (dbgStat)
 {
  Serial.print("x = ");
  Serial.print(xValue);
  Serial.print(", y = ");
  Serial.println(yValue);
  Serial.print("count = ");
  Serial.println(count);
 }
}

 if (dbgStat)
 {
 smDebugIn();
 }

}


void menuHome() {
    lcd.clear();
    lcd.print("Home Screen");
    lcd.setCursor(0, 1);
    lcd.print("Use JS to NAV");
}


void menuFirst() {
    lcd.clear();
    lcd.print("     First    ");
    lcd.setCursor(0, 1);
    lcd.print("First Top Menu");

}

void menuSub1First() {
  lcd.clear();
  lcd.print(" Sub Menu ");
  lcd.setCursor(0, 1);
  lcd.print("First Menu Sub");
  
}

void menuSecond() {
  lcd.clear();
  lcd.print("    Second");
  lcd.setCursor(0, 1);
  lcd.print("2nd Top Menu");
}

//sub menu for 2nd menu
void menuSub1Second(){
  
    lcd.clear();
    lcd.print("Debug Mode");
    lcd.setCursor(0,1);
    if (dbgStat==0)
    {
    lcd.print("OFF");
    } else
    {
    lcd.print("ON");
    }

}

void menuSub2Second() {
  lcd.clear();
  lcd.print(" Second Sub");
  lcd.setCursor(0, 1);
  lcd.print("2nd Sub Menu");
}

  
void smDebugIn() {
  if (Serial.available()) { 
   char input = Serial.read();
  if (input ==  'a') { menuHome();}
  if (input == 'b') {menuFirst();}
  if (input == 'c') {menuSecond();}
  }
}