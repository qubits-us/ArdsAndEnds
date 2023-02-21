/*
qubits.us
Async Morse Code Lib.
Created 2.21.2023 ~q

Free source for all..
No warranties, use at yee own risk..
*/

#include "AsyncMorse.h"
#include <Arduino.h>


AsyncMorse::AsyncMorse(int pin){
Morse_Pin = pin;
}

//clear out the old
void AsyncMorse::ZeroMorseBuff()
{
  for (int i = 0; i < sizeof(morseBuff); i++ ) {
    morseBuff[i] = 0;
  }
}



void AsyncMorse::MorseLedOn()
{
  digitalWrite(Morse_Pin,HIGH);

}
void AsyncMorse::MorseLedOff()
{
  digitalWrite(Morse_Pin,LOW);

}


void AsyncMorse::MorseStop()
{
  saved_loop = loop_morse;
  loop_morse = false;
  morse_done = true;
}

void AsyncMorse::MorseStart()
{
   if (morse_loaded){
  loop_morse = saved_loop;
  morse_done = false;
   }
}

void AsyncMorse::MorseRestart()
{
   if (morse_loaded){
        mc_done = false;
        dd_done = true;
        dd_idx = 0;
        mc_idx = 0;
        mc_state = 0;
        dd_state = 0;
        morse_done = false;
}
}



bool AsyncMorse::MorseString(char *text, bool looped) {
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
    morse_loaded = true;
    mc_count = count;
    Serial.print("chars loaded: ");
    Serial.println(count);
    Serial.println(morseBuff);
    Serial.print(morseBuff[mc_idx]);
    morse_done = false;
  }
  return loaded;
}



void AsyncMorse::MorseLoop() {
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
void AsyncMorse::MorseChar(byte morse_code, bool digit) {
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

byte AsyncMorse::GetDit(byte morse_code, bool digit) {
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


void AsyncMorse::DitDat(byte dit) {
  if (dd_state == 0) {
    MorseLedOn();
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
      MorseLedOff();
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