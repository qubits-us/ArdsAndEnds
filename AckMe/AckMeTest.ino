/*
  AckMe Tester Arduino Mega
  Using Serial1 - PC
  Serial for programming and debugging..
*/

#define ACK 6
#define NAK 21
const unsigned long baud = 9600;
const unsigned int bufSize = 256;
const char delimiter = ';';

char buf[bufSize];
int recvd = 0;

void setup() {
  while (!Serial) { delay(10); }
  Serial.begin(9600);
  Serial.println("booting..");
  Serial1.begin(baud);
  Serial.println("Ready..");
}

void loop() {
  if (Serial1.available()) {
    char aByte = Serial1.read();
    Serial.write(aByte);
    if (aByte != delimiter) {
      buf[recvd] = aByte;
      recvd++;
      if (recvd >= sizeof(buf)) {
        //overflowing..
        Serial.println("Overflow, increase buffer size..");
        recvd = 0;
        memset(buf, 0, sizeof(buf));
        Serial1.flush();
      }
    } else {
      //got a new packet..
      Serial.println("");
      Serial.print("Recvd Packet Size:");
      Serial.println(recvd);
      Serial.print("buffer:");
      Serial.write(buf,recvd);
      Serial.println("");
      recvd = 0;
      memset(buf, 0, sizeof(buf));
      //simulate some lengthy processing..
      delay(1000);
      //send ack to get next packet..
      Serial1.write(ACK);
    }
  }
}