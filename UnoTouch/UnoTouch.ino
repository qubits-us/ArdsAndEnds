/*
  ILI9341 Cap Touch Scrren Multiple Screen navigation demo..
  simple demo with 1 home screen and 4 other screens on an uno..
  DrawCircle make a simple circle button with centered text..
  DrawButton makes a simple button with centered text..
  TouchArea returns true if the area was touched note on..
  this one, touch cords are off in demo, so i flipped them in function..
  CheckScreenTouch primary touch loop for all screens..

  created 2.18.2023 ~q

*/



#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Adafruit_ILI9341.h>
#include <Arduino.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>



#define BLACK 0x0000  // macros for color (16 bit)
#define NAVY 0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LIGHTGREY 0xC618
#define DARKGREY 0x7BEF
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GREENYELLOW 0xAFE5
#define PINK 0xF81F
#define LIME 0x67E0

//nano defines
#define LCD_CS 10     // Chip Select goes to Analog 3
#define LCD_CD 9     // Command/Data goes to Analog 2
#define LCD_WR 12     // LCD Write goes to Analog 1MOSI
#define LCD_RD 11     // LCD Read goes to Analog 0
//#define LCD_RESET A4  // Can alternately not connect at all..

/* Mega Defines
  #define LCD_CS A3     // Chip Select goes to Analog 3
  #define LCD_CD A2     // Command/Data goes to Analog 2
  #define LCD_WR A1     // LCD Write goes to Analog 1
  #define LCD_RD A0     // LCD Read goes to Analog 0
  #define LCD_RESET A4  // Can alternately just connect to Arduino's reset pin
*/

//Create some objects
Adafruit_FT6206 ts = Adafruit_FT6206();//touch
Adafruit_ILI9341 tft = Adafruit_ILI9341(LCD_CS, LCD_CD);//screen


//current screen we are on..
int CurrentScreen = 0;
//debounce the touch screen
unsigned long lastTouchMilli = 0;
//check touch once per
int intervalTouch = 10;//miliis
//are we debugging?
bool Debugging = true;
//keep track of our circle rads..
int CircleRad = 45;


void setup() {

  Serial.begin(115200);
  while (!Serial);  

  // tft.begin(0x9341);  // Initialise LCD
  tft.begin();
  if (Debugging) Serial.println("Starting Touch");
  if (! ts.begin(10)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }
   ScreenHome();
}



void loop() {

    //debounce touch screen once per interval
    if (millis() - lastTouchMilli >= intervalTouch)
    { lastTouchMilli = millis();
    CheckScreenTouch();
    }
}

void ScreenHome()
{
  CurrentScreen = 0;
  tft.fillScreen(BLACK);
  tft.setTextSize(2);
  DrawButton("",2,2,238,62,BLACK,LIME);
  DrawText("Home Screen", 10, 8, WHITE);
  DrawText("Menu Demo", 10, 35, WHITE);
  DrawButton("Screen 1",0, 64, 240, 64, LIGHTGREY,DARKGREY);
  DrawButton("Screen 2",0, 128, 240, 64, LIGHTGREY,DARKGREY);
  DrawButton("Screen 3",0, 192, 240, 64, LIGHTGREY,DARKGREY);
  DrawButton("Screen 4",0, 256, 240, 64, LIGHTGREY,DARKGREY);
}



void ScreenOne()
{
  CurrentScreen = 1;
  tft.fillScreen(BLACK);
  tft.setTextSize(2);
  DrawText("Screen 1", 40, 4, WHITE);
  tft.setTextSize(3);  
  DrawCircle("AA",110, 90, CircleRad,LIGHTGREY,DARKGREY);
  DrawCircle("BB",110, 190, CircleRad,LIGHTGREY,DARKGREY);
  DrawButton("Back",0, 255, 100, 50,LIGHTGREY,DARKGREY);
}

void ScreenTwo()
{
 CurrentScreen = 2;
 tft.fillScreen(BLACK);
 tft.setTextSize(2);
 int w = tft.width();
 int h = tft.height();
 int cols = 3;
 int rows = 3;
 int gap = 2;
 int btnWidth = (w / cols) - (gap *(cols+1));
 int border = w - ((btnWidth*cols)+(gap*(cols+1)));
 int btnHeight = (h / (rows+1)) - (gap*(rows+2));
 int x = gap;
 int y = border;
 char btnTxt[2];
 int btnNum = 0;
   for (int i = 0;i<rows;i++)
   {x = gap+ (border /2);
     for (int j=0;j<cols;j++)
     { btnTxt[0] = char(49+btnNum);
       btnNum++;
       DrawButton(btnTxt,x,y,btnWidth,btnHeight,LIGHTGREY,DARKGREY);
       x+=btnWidth+gap;
     }
    y+=btnHeight+gap;
   }
  DrawButton("Back",0, 255, 100, 50,LIGHTGREY,DARKGREY);
}

void ScreenThree()
{
  CurrentScreen = 3;
  tft.fillScreen(BLACK);
  tft.setTextSize(2);
  DrawText("Screen 3", 40, 4, WHITE);
  DrawButton("Back",0, 255, 100, 50,DARKGREY, LIGHTGREY);
}

void ScreenFour()
{
  CurrentScreen = 4;
  tft.fillScreen(BLACK);
  tft.setTextSize(2);
  DrawText("Screen 4", 40, 4, WHITE);
  DrawButton("Back",0, 255, 100, 50,DARKGREY, LIGHTGREY);
}


void DrawButton(const char *text,int16_t x, int16_t y, int16_t w, int16_t h, int16_t color, int16_t bordercolor) {
  tft.fillRoundRect(x, y, w, h,10, bordercolor);
  tft.fillRoundRect(x+2, y+2, w-4, h-4,10, color);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  int16_t x1,y1;
  uint16_t h1,w1;
  h1=0;
  w1=0;
  //get the w and h of our text
  tft.getTextBounds(text,x,y,&x1,&y1,&w1,&h1);  
  //center it..
  tft.setCursor(x+w/2-w1/2, y+h/2-h1/2);
  tft.println(text);
}


void DrawCircle(char *text, int16_t x, int16_t y, int16_t r, int16_t color, int16_t bordercolor) {
  tft.fillCircle(x, y, r, bordercolor);
  tft.fillCircle(x, y, r - 5, color);
  int16_t x1,y1;
  uint16_t h1,w1;
  h1=0;
  w1=0;
  //get the w and h of our text
  tft.getTextBounds(text,x,y,&x1,&y1,&w1,&h1);  
  //center it..
  tft.setCursor(x-w1/2, (y-(r/2))+h1/2);
  tft.println(text);
}

//combines text and color
void DrawText(const char *text, int16_t x, int16_t y, int16_t color) {
  tft.setTextColor(color);  // Set Text Proporties
  tft.setCursor(x, y);
  tft.println(text);
}


// are we touching the area??
//pass in the points touched and area to match..
bool TouchArea( int px, int py, int x, int y, int w, int h)
{

  int x2 = x + w;
  int y2 = y + h;
  bool match = false;
  //manipulate points, if needed..
  int touch_x = (240 - px);
  int touch_y =  (320 - py);

  if ((touch_x >= x) && (touch_x <= x2) && (touch_y >= y) && (touch_y <= y2))
  {
    match = true;
  }

  if (Debugging){
  Serial.print("X = ");
  Serial.print(touch_x);
  Serial.print(", X1 = ");
  Serial.print(x);
  Serial.print(", Y = ");
  Serial.print(touch_y);
  Serial.print(", Y1 = ");
  Serial.print(y);
  Serial.print(", match = ");
  Serial.println(match);}

  return match;
}

//check touch for all screens..
void CheckScreenTouch()
{
  if (ts.touched()) { 
    //need to reuse the x y for all button checks..
    TS_Point p = ts.getPoint();
    if (Debugging){
    Serial.print("Checking buttons Screen: ");
    Serial.println(CurrentScreen);}

    switch (CurrentScreen){
      case 0: //home screen
       if (TouchArea(p.x, p.y, 0, 64, 240, 64)) {
            if (Debugging) Serial.println("Button 1");
             ScreenOne();
          } else if (TouchArea(p.x, p.y, 0, 128, 240, 64)) {
             ScreenTwo();
           if (Debugging) Serial.println("Button 2");
          } else if (TouchArea(p.x, p.y, 0, 192, 240, 64)) {
             ScreenThree();
           if (Debugging) Serial.println("Button 3");
          } else if (TouchArea(p.x, p.y, 0, 256, 240, 64)) {
            ScreenFour();
           if (Debugging) Serial.println("Button 4");
          } 
           break;
      case 1: //Screen 1
        if (TouchArea(p.x, p.y, 0, 190-CircleRad/2, 240, CircleRad*2))  {
           if (Debugging) Serial.println("Button 1");
          } else if (TouchArea(p.x, p.y, 0, 90-CircleRad/2, 240, CircleRad*2)) {
           if (Debugging) Serial.println("Button 2");
          } else if (TouchArea(p.x, p.y, 0, 255, 240, 50)) {
           if (Debugging) Serial.println("Button Back");
           ScreenHome();
          } 
          break;     
      case 2: //Screen 2
           if (CheckTouchNumPad(p.x,p.y)){ 
           if (Debugging) Serial.println("Num Pad");
           } else
           if (TouchArea(p.x, p.y, 0, 255, 240, 50)) {
           if (Debugging) Serial.println("Button Back");
           ScreenHome();
          } 
          break;     
      case 3: //Screen 3
           if (TouchArea(p.x, p.y, 0, 255, 240, 50)) {
           if (Debugging) Serial.println("Button Back");
           ScreenHome();
          } 
          break;     
      case 4: //Screen 4
           if (TouchArea(p.x, p.y, 0, 255, 240, 50)) {
           if (Debugging) Serial.println("Button Back");
           ScreenHome();
          } 
          break;     
    }
  }
}

//check touches on the number pad screen 2..
bool CheckTouchNumPad(int px, int py)
{
 int w = tft.width();
 int h = tft.height();
 int cols = 3;
 int rows = 3;
 int gap = 2;
 int btnWidth = (w / cols) - (gap *(cols+1));
 int border = w - ((btnWidth*cols)+(gap*(cols+1)));
 int btnHeight = (h / (rows+1)) - (gap*(rows+2));
 int x = gap;
 int y = border;
 int btn = 1;
 bool click = false;
   for (int i = 0;i<rows;i++)
   {x = gap+ (border /2);
     for (int j=0;j<cols;j++)
     { 
      if (TouchArea(px,py,x,y,btnWidth,btnHeight)){
        click = true;
        break;
      }
       x+=btnWidth+gap;
       btn++;
     }
     if (click) break;
    y+=btnHeight+gap;
   }

   if (click){
       switch (btn){
         case 1:if (Debugging) Serial.println("BTN1");break;
         case 2:if (Debugging) Serial.println("BTN2");break;
         case 3:if (Debugging) Serial.println("BTN3");break;
         case 4:if (Debugging) Serial.println("BTN4");break;
         case 5:if (Debugging) Serial.println("BTN5");break;
         case 6:if (Debugging) Serial.println("BTN6");break;
         case 7:if (Debugging) Serial.println("BTN7");break;
         case 8:if (Debugging) Serial.println("BTN8");break;
         case 9:if (Debugging) Serial.println("BTN9");break;
       }
   }

   return click;

}