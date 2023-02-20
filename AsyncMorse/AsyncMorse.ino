/*
  Async Morse Code..

  Non-blocking morse code..
  MorseChars array of all alphas and digits..
  1st 26 bytes are A-Z.. high nibble is a counter, contains how many dits or dats, low nibble is dit pattern 0=dit 1=dat..
  last 10 bytes are 0-9.. numbers are all 5 bit dit patterns..

  created 2.20.2023 ~q
*/

#include <Adafruit_NeoPixel.h>

#define PIN 7 // Data pin number for the NeoPixels
#define NUMPIXELS 16 // Define how many pixels to use
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800 );

uint32_t red = pixels.Color(255, 63, 5);
uint32_t lightred = pixels.Color(140, 0, 0);
uint32_t off = pixels.Color(0, 0, 0);


//alphas A-Z
byte MorseChars[] = {34, 65, 69, 49, 16, 68, 51, 64, 32, 78, 53, 66, 35, 33, 55, 70, 75, 50, 48, 17, 52, 72, 54, 73, 77, 67,
                     31, 30, 28, 24, 16, 0, 1, 3, 7, 15
                    };
//digits 0-9

char morseBuff[80];
int mc_state = 0;
int mc_count = 0;
int mc_idx = 0;
int dd_state = 0;
int dd_idx = 0;
int dd_time = 0;
int dot_dur = 200;
int dd_dur = 200;
bool mc_done;
bool dd_done;
bool morse_done = true;
bool loop_morse = false;
unsigned long dd_start;

void setup() {
  pixels.begin(); // Initialize the NeoPixel library.
  pixels.show();
  pixels.setBrightness(255);
  Serial.begin(9600);
  Serial.println("Begin");
  MorseString("SOS SOS SOS", true);
}
void loop() {

  MorseLoop();

}

void leds_on()
{
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, red);
  }
  pixels.show();

}
void leds_off()
{
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, off);
  }
  pixels.show();

}

//clear out the old
void ZeroMorseBuff()
{
  for (int i = 0; i < sizeof(morseBuff); i++ ) {
    morseBuff[i] = 0;
  }
}


bool MorseString(char *text, bool looped) {
  loop_morse = looped;
  bool loaded = false;
  if (strlen(text) > 0) {
    Serial.println(strlen(text));
    mc_idx = 0;
    mc_done = false;
    mc_state = 0;
    dd_done = true;
    dd_idx = 0;
    dd_state = 0;
    ZeroMorseBuff();
    char ch = *text;
    int count = 0;
    while (ch != NULL)
    {
      morseBuff[count] = ch;
      count++;
      *text++;
      ch = *text;
    }
    loaded = true;
    mc_count = count;
    Serial.print("chars loaded: ");
    Serial.println(count);
    Serial.println(morseBuff);
    Serial.print(morseBuff[mc_idx]);
    morse_done = false;
  }
  return loaded;
}



void MorseLoop() {
  if (!morse_done) {
    if (mc_done) {
      if (mc_idx < mc_count) {
        mc_idx ++;
        if (mc_idx < mc_count) {
          mc_done = false;
          mc_state = 0;
          dd_done = true;
          dd_idx = 0;
          dd_state = 0;
          Serial.println();
          Serial.print(morseBuff[mc_idx]);
          if (isalpha(morseBuff[mc_idx]))
          { if (isupper(morseBuff[mc_idx]))
            {
              MorseChar(MorseChars[morseBuff[mc_idx] - 'A'], false);
              //morse_char(letters[morseBuff[mc_idx] - 'A']);
            } else
            {
              MorseChar(MorseChars[morseBuff[mc_idx] - 'a'], false);
            }
          } else if (isdigit(morseBuff[mc_idx])) {
            MorseChar(MorseChars[morseBuff[mc_idx] - '1' + 27], true);
          } else if (morseBuff[mc_idx] == ' ') {
            Serial.println(":Space");
            mc_state = 1;
            dd_start = millis();
            dd_dur = dot_dur * 3;
          } else mc_done = true;//skip it
        } else {
          dd_start = millis();
          dd_dur = dot_dur * 6;
          morse_done = true;
        }
      } else {
        dd_start = millis();
        dd_dur = dot_dur * 6;
        morse_done = true;
      }
    } else
    {
      if (isalpha(morseBuff[mc_idx]))
      { if (isupper(morseBuff[mc_idx]))
        {
          MorseChar(MorseChars[morseBuff[mc_idx] - 'A'], false);
          //morse_char(letters[morseBuff[mc_idx] - 'A']);
        } else
        {
          MorseChar(MorseChars[morseBuff[mc_idx] - 'a'], false);
        }

      } else if (isdigit(morseBuff[mc_idx])) {
        MorseChar(MorseChars[morseBuff[mc_idx] - '1' + 27], true);
      } else if (morseBuff[mc_idx] == ' ') {
        MorseChar(MorseChars['B' - 'A'], false);
        //morse_char(letters['B' - 'A']);
      }
    }
  } else
  {
    if (loop_morse) {
      if (millis() - dd_start >= dd_dur) {
        mc_done = false;
        dd_done = true;
        dd_idx = 0;
        mc_idx = 0;
        mc_state = 0;
        dd_state = 0;
        morse_done = false;
        Serial.println();
        Serial.println("looping-");
        Serial.print(morseBuff[mc_idx]);
      }
    }
  }
}




//starts a char morsing..
void MorseChar(byte morse_code, bool digit) {
  if (mc_state == 0) {
    byte dit = GetDit(morse_code, digit);
    if (dd_done) {
      if (dit != 99) {
        if (dd_idx == 0) dd_state = 0;//beginning
        dd_done = false;
        dd_state = 0;
        DitDat(dit);
        dd_idx ++;
      } else {
        // mc_done = true;
        dd_idx = 0;
        mc_state = 1;//inter char delay
        dd_start = millis();
        dd_dur = dot_dur * 3;
      }
    } else {
      DitDat(dit);
    }
  } else { //mc_state = 1 = inter char delay
    if (millis() - dd_start >= dd_dur) {
      mc_done = true;
    }
  }
}

byte GetDit(byte morse_code, bool digit) {
  byte dit = 99;// all done
  //digits are fixed count of 5
  int bits = 0;
  if (digit) bits = 5 ; else bits = morse_code >> 4;
  if (dd_idx == 0) dd_state = 0;//beginning
  if (bits > 0 && bits < 6 && dd_idx < bits) {
    dit = (morse_code >> dd_idx) & 0x1;
  }
  return dit;
}


void DitDat(byte dit) {
  if (dd_state == 0) {
    leds_on();
    dd_state = 1;
    dd_start = millis();
    if (dit == 0) {
      dd_dur = dot_dur;
      Serial.print(".");
    } else {
      dd_dur = dot_dur * 3;
      Serial.print("-");
    }
  } else if (dd_state == 1) {
    if (millis() - dd_start >= dd_dur) {
      dd_state = 2;
      leds_off();
      dd_start = millis();
      dd_dur = dot_dur;
    }
  } else if (dd_state == 2) {
    if (millis() - dd_start >= dd_dur) {
      dd_state = 3;
      dd_done = true;
    }

  }
}