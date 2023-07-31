# include <Adafruit_NeoPixel.h>


#define LED_PIN    5
#define LED_COUNT 32
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


int16_t cur_bright = 255;

//vars used for rainbow
long CurrentPixelHue = 0;
int  CurrentPixel = 0;
int CurrentAni = 0;
unsigned long msecLastAniStep = 0;
int AniStepDelay = 250;
uint32_t CurrentColor = 0;
long RainBowDone = (5 * 65536);
bool AniDone = true;
//Theater
int pixelQueu = 0;
int pixelCycle = 0;



void setup()
{
  Serial.begin(9600);
  Serial.println("Ready");
  strip.begin();
  strip.show();
  strip.setBrightness(255);
}



void loop(void)

{


  PumpAni();



  if (Serial.available ())  {
    char c = Serial.read ();
    switch (c)   {
      case 'z':
        AniDone = false;
        CurrentPixelHue = 0;
        CurrentPixel = 0;
        rainbow(2);
        break;
      case 'r':
        Serial.println("RED");
        AniDone = false;
        CurrentPixel = 0;
        colorWipe (strip.Color(255, 0, 0), 20);
        break;
      case 'g':
        AniDone = false;
        CurrentPixel = 0;
        colorWipe (strip.Color(0, 255, 0), 20);
        break;
      case 'b':
        AniDone = false;
        CurrentPixel = 0;
        colorWipe (strip.Color(0, 0, 255), 20);
        break;
      case 'w':
        AniDone = false;
        CurrentPixel = 0;
        colorWipe (strip.Color(255, 255, 255), 20);
        break;
      case 'y':
        AniDone = false;
        CurrentPixel = 0;
        colorWipe (strip.Color(255, 255, 0), 20);
        break;
      case 'o':
        AniDone = false;
        CurrentPixel = 0;
        colorWipe (strip.Color(255, 0, 255), 20);
        break;
      case 't':
        AniDone = false;
        CurrentPixel = 0;
        pixelQueu = 0;
        colorWipe (strip.Color(0, 255, 255), 20);
        break;
      case 'a':
        AniDone = false;
        CurrentPixel = 0;
        pixelQueu = 0;
        theaterChase(strip.Color(0, 255, 255), 20);
        break;
      case 'q':
        Serial.println("TheaterChaseRainbow");
        AniDone = false;
        CurrentPixel = 0;
        pixelQueu = 0;
        pixelCycle = 0;
        theaterChaseRainbow(20);
        break;
      case 'p':
        Serial.println("Rainbow Fill");
        AniDone = false;
        CurrentPixel = 0;
        pixelQueu = 0;
        pixelCycle = 0;
        rainbowFill(20);
        break;
      case '.':

        break;
      case ',':

        break;
      case '+':
        cur_bright = cur_bright + 25;
        if (cur_bright > 255)
          cur_bright = 255;
        strip.setBrightness(cur_bright);
        break;
      case '-':
        cur_bright = cur_bright - 25;
        if (cur_bright < 0)
          cur_bright = 10;
        strip.setBrightness(cur_bright);
        break;

      default:
        break;
    }
  }
}


void PumpAni ()
{ //see if it's time for a step..
  unsigned long currMilli = millis();

  if (currMilli - msecLastAniStep >= AniStepDelay)
  {
    msecLastAniStep = currMilli;

    switch (CurrentAni) {
      case 0: colorWipe(CurrentColor, AniStepDelay); break;
      case 1: rainbow(AniStepDelay); break;
      case 2: theaterChase(CurrentColor, AniStepDelay); break;
      case 3: theaterChaseRainbow(AniStepDelay); break;
      case 4: rainbowFill(AniStepDelay); break;
    }
  }
}

void rainbowFill(int wait) {

  AniStepDelay = wait;
  CurrentAni = 1;

  if (!AniDone)
  {
    int pixelHue = CurrentPixelHue + (CurrentPixel * 65536L / strip.numPixels());
    strip.setPixelColor(CurrentPixel, strip.gamma32(strip.ColorHSV(pixelHue)));
    strip.show();
  }

  if (CurrentPixel < strip.numPixels())
  {
    CurrentPixel++;
  }
  else
  {
    CurrentPixel = 0;
    AniDone = true;
  }

}





void rainbow(int wait) {

  AniStepDelay = wait;
  CurrentAni = 1;

  if (CurrentPixel < strip.numPixels())
  {
    CurrentPixel++;
  }
  else
  {
    CurrentPixel = 0;
    if (CurrentPixelHue < RainBowDone) {
      CurrentPixelHue += 256;
    } else {
      AniDone = true;
    }
  }
  if (!AniDone)
  {
    int pixelHue = CurrentPixelHue + (CurrentPixel * 65536L / strip.numPixels());
    strip.setPixelColor(CurrentPixel, strip.gamma32(strip.ColorHSV(pixelHue)));
    strip.show();
  }
}

void colorWipe(uint32_t color, unsigned long wait)
{
  CurrentAni = 0;
  AniStepDelay = wait;
  CurrentColor  = color;

  if (!AniDone)
  {
    strip.setPixelColor(CurrentPixel, CurrentColor);
    strip.show();
  }

  if (CurrentPixel < strip.numPixels())
  {
    CurrentPixel++;
  } else
  {
    CurrentPixel = 0;
    AniDone = true;
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  CurrentAni = 2;
  CurrentColor = color;

  if (AniDone) return;

  if (AniStepDelay != wait)
    AniStepDelay = wait;//  Update delay time

  strip.setPixelColor(CurrentPixel + pixelQueu, CurrentColor); 
  strip.show();                             

  for (int i = 0; i < strip.numPixels(); i += 3) {
    strip.setPixelColor(i + pixelQueu, strip.Color(0, 0, 0)); 
  }
  pixelQueu++;                        
  if (pixelQueu >= 3)
    pixelQueu = 0;


  if (CurrentPixel < strip.numPixels())
  {
    CurrentPixel++;
  } else
  {
    CurrentPixel = 0;
    AniDone = true;
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  CurrentAni = 3;
  if (AniDone) return;
  if (AniStepDelay != wait)
    AniStepDelay = wait;                  
  for (int i = 0; i < strip.numPixels(); i += 3) {
    strip.setPixelColor(i + pixelQueu, Wheel((i + pixelCycle) % 255)); 
  }
  strip.show();
  for (int i = 0; i < strip.numPixels(); i += 3) {
    strip.setPixelColor(i + pixelQueu, strip.Color(0, 0, 0)); 
  }
  pixelQueu++;                          
  pixelCycle++;                         
  if (pixelQueu >= 3)
    pixelQueu = 0;                   
  if (pixelCycle >= 256)
    pixelCycle = 0;                     

  if (CurrentPixel < strip.numPixels())
  {
    CurrentPixel++;
  } else
  {
    CurrentPixel = 0;
    AniDone = true;
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}