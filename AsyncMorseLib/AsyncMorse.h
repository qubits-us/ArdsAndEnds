/*
qubits.us
Async Morse Code Lib.
Created 2.21.2023 ~q

Free source for all..
No warranties, use at yee own risk..
*/

#ifndef AsyncMorse_h
#define AsyncMorse_h

#include <Arduino.h>

const byte MorseChars[] = {34, 65, 69, 49, 16, 68, 51, 64, 32, 78, 53, 66, 35, 33, 55, 70, 75, 50, 48, 17, 52, 72, 54, 73, 77, 67, 31 , 30, 28, 24, 16, 0, 1, 3, 7, 15};

class AsyncMorse {

private:
  int Morse_Pin;
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
  bool saved_loop = false;
  bool morse_loaded = false;  
  unsigned long dd_start;
  void ZeroMorseBuff();
  void MorseChar(byte morse_code, bool digit);
  byte GetDit(byte morse_code, bool digit);
  void DitDat(byte dit);
  void MorseLedOn();
  void MorseLedOff();

  

public:
       AsyncMorse(int pin);
  bool MorseString(char *text, bool looped);
  void MorseLoop();
  void MorseStop();
  void MorseStart();
  void MorseRestart();
};

#endif
