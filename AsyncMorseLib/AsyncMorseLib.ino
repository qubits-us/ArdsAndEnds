#include "AsyncMorse.h"



#define LED_PIN  LED_BUILTIN

AsyncMorse Morse(LED_PIN);

void setup()
{

pinMode(LED_PIN,OUTPUT);
Morse.MorseString("SOS",true);

}


void loop()
{

Morse.MorseLoop();

}