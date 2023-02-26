/*
  Small serial command processor..
  allows for one int param to be passed in..
  ex. SPEED=100
  command SPEED would be looked up and NewValue would equal 100

created 2.25.2023 ~q

*/




//how many command
const byte NumCommands = 2;
//the commands to watch for
const char* Commands[] = {"LIST", "HELP"};
//buffer to store incomming
char buff[80];
//new value sent in XXX=100
int NewValue = 0;
//have we recv a command
bool CommandRecv = false;
//keep track of where we are
int CharCounter = 0;
void setup() {
  
  Serial.begin(115200);
  Serial.println("Ready");


}

void loop() {

//check for commands..
CheckForCommand();

}


void CheckForCommand(){
  while (Serial.available())
  {
    char achar = Serial.read();
    if (achar != 10) {
      if (CharCounter < sizeof(buff)) {
        buff[CharCounter] = achar;
        CharCounter++;
      }
    } else CommandRecv = true;
  }


   //process incoming command..
   //use a switch statement for many commands..
  if (CommandRecv) {
    CommandRecv = false;
    CharCounter = 0;
    Serial.println(buff);
    Serial.println(ParseCommand(buff));
    ZeroBuffer();
  }
}

//splits incoming string
//into command and value
//returns the commands array position if found
//returns -1 if command was not found..
int ParseCommand(char* abuff) {

  int CommandNum = -1;
  int splits = 0;
  char* comm;
  char* value;

  for ( char* piece = strtok( abuff, "=");
        piece != nullptr;
        piece = strtok( nullptr, "=")) {
    if (splits == 0) {
      comm = piece;
      splits++;
    } else if (splits == 1) {
      value = piece;
      splits++;
    }
  }

  //convert and store our value
  if (value != nullptr) {
    NewValue = atoi(value);
  }

  if (comm != nullptr) {
    for (int i = 0; i < NumCommands; i++) {
      if (strcmp(Commands[i], comm) == 0) {
        CommandNum = i;
        break;
      }
    }
  }
  return CommandNum;
}

//empties out the old..
void ZeroBuffer() {
  for (int i = 0; i < sizeof(buff); i++) {
    buff[i] = 0;
  }

}