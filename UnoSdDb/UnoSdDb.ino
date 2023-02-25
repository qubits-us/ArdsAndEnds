/*
  Example of using SD card to keep small DB
  Fixed size records..
  Currently just a circular buffer..

  2.25.2023 ~q

*/



#include <SD.h>

#define CS_PIN 10
//How Many Records to store in DB
#define MAX_RECS  5

#define BTN_SAMPLE 3
//random write mode
#define FILE_RANDOM_WRITE (O_WRITE | O_READ | O_CREAT)

//The Data to store
struct __attribute__((__packed__)) {
  int16_t sequence = 0;//used in function KEEP
  int16_t temp = 0;
  int16_t humidity = 0;
} SampleRec;


File root;
File myFile;

uint8_t buff[100];

uint16_t NextSequence = 1;
uint16_t CurrRec = 0;

unsigned long lastSample = 0;
int intervalSample = 20;

char command[80];
int CharCount = 0;
bool HaveCommand = false;

void setup() {
  Serial.begin(115200);

  Serial.print("Initializing SD card... ");

  if (!SD.begin(CS_PIN)) {
    Serial.println("Card initialization failed!");
    while (true);
  }

  Serial.println("initialization done.");

  Serial.println("Files in the card:");
  root = SD.open("/");
  printDirectory(root, 0);
  Serial.println("");



  NextSequence = LoadSequence();
  SampleRec.sequence = NextSequence;
  SampleRec.temp = 80;
  SampleRec.humidity = 60;
  SaveSample();
  SampleRec.sequence = NextSequence;
  SaveSample();

  Serial.println("Files in the card:");

  root = SD.open("/");
  printDirectory(root, 0);
  Serial.println("");
  pinMode(BTN_SAMPLE, INPUT_PULLUP);


}



bool Saving = false;
void loop() {
  // nothing happens after setup finishes.
  if (millis() - lastSample >= intervalSample) {
    lastSample = millis();
    if (!digitalRead(BTN_SAMPLE) && !Saving)
    {
      Saving = true;
      SampleRec.sequence = NextSequence;
      SampleRec.temp = 81;
      SampleRec.humidity = 66;
      SaveSample();
      Serial.println("Files in the card:");
      root = SD.open("/");
      printDirectory(root, 0);
      Serial.println("");

    } else if (digitalRead(BTN_SAMPLE)) Saving = false;

  }


  while (Serial.available())
  {
    char ch = Serial.read();
    if (ch != 10) {
      if (CharCount < sizeof(command)) command[CharCount] = ch;
      CharCount++;
    } else HaveCommand = true;
  }
  if (HaveCommand) {
    switch (command[0]) {
      case 'L': Serial.println("Files in the card:");
        root = SD.open("/");
        printDirectory(root, 0);
        Serial.println("");
        break;
      case 'C': Serial.print("Records: "); Serial.println(CountSamples()); break;
      case '1': LoadSample(0); Serial.println("Sample 1"); Serial.print("Seq:");
        Serial.println(SampleRec.sequence); break;
      case '2': LoadSample(1); Serial.println("Sample 2"); Serial.print("Seq:");
        Serial.println(SampleRec.sequence); break;
      case '3': LoadSample(2); Serial.println("Sample 3"); Serial.print("Seq:");
        Serial.println(SampleRec.sequence); break;
      case '4': LoadSample(3); Serial.println("Sample 4"); Serial.print("Seq:");
        Serial.println(SampleRec.sequence); break;
      case '5': LoadSample(4); Serial.println("Sample 5"); Serial.print("Seq:");
        Serial.println(SampleRec.sequence); break;
    }
    HaveCommand = false;
    CharCount = 0;
  }
}


void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}


//how many records..
int CountSamples() {
  int count = 0;
  myFile = SD.open("sample.db", FILE_READ);
  if (myFile)
  {
    if (myFile.size() > 0) {
      count = myFile.size() / sizeof(SampleRec);
    }
    myFile.close();
  }
  return count;
}

//loads the next sequence
//for start up..
int LoadSequence() {
  int seq = 0;
  int count = CountSamples();
  if (count > 0) {
    for (int i = 0; i < count ; i++) {
      if (LoadSample(i)) {
        if (SampleRec.sequence > seq) {
          seq = SampleRec.sequence;
          CurrRec = i + 1;
        }
      } 
    }
  }
  return seq + 1;
}


bool LoadSample(int recNo) {
  int count = 0;
  bool GoodRead = false;
  myFile = SD.open("sample.db", FILE_READ);
  if (myFile)
  {
    if (myFile.size() > 0) {
      if ( recNo <= myFile.size() / sizeof(SampleRec)) {
        myFile.seek(recNo * sizeof(SampleRec));
        int bytesRead = myFile.read(buff, sizeof(SampleRec));
        if (bytesRead == sizeof(SampleRec)) {
          memcpy(&SampleRec, &buff, sizeof(SampleRec));
          GoodRead = true;
        } 
      } 
    }
    myFile.close();
  } else Serial.println("File failed to open");
  return GoodRead;
}

bool SaveSample() {
  myFile = SD.open("sample.db", FILE_RANDOM_WRITE);
  if (myFile)
  {
    if (myFile.size() < sizeof(SampleRec)* MAX_RECS)
    {
      int NewPos = myFile.seek(myFile.size());

    } else
    { if (CurrRec == MAX_RECS) CurrRec = 0;
      int NewPos = myFile.seek(sizeof(SampleRec) * CurrRec);
    }
    myFile.write((uint8_t *)&SampleRec, sizeof(SampleRec));
    myFile.close();
    myFile.flush();
    NextSequence++;
    CurrRec++;
    return true;
  } else return false;
}


