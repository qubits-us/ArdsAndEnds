//UDP Receive..


#include <WiFi.h>
#include <WiFiUdp.h>

// network SSID (network name). and network password.
char ssid[] = "****";
char pass[] = "****";



// network key Index number (needed only for WEP).
int keyIndex = 0;

// UDP information
//local broadcast address..
IPAddress udpServer(192, 168, 0, 255);
#define UDP_PORT 3000
WiFiUDP udp;

// UDP Datagram
struct __attribute__((packed)) UDPDatagram {
  uint16_t seq;                          // sequence number
  int16_t sdata;                         // 16 bit integer data
  int32_t idata;                         // 32 bit integer data
  float fdata;                           // float data
  uint16_t crc;                          // crc check
} udpDatagram = { 0, 0, 0, 0.0, 0 };  // initial values


uint8_t buff[sizeof(UDPDatagram)];

void setup() {
  Serial.begin(115200);
  // Wait for serial port to connect. Needed for Leonardo only
  while (!Serial)
    ;
  delay(1000);
  Serial.println();
  Serial.println("ESP32 WiFi UDP listener - receives UDP datagrams from sender");
  WiFi.mode(WIFI_STA);  // Connect to Wifi network.
  WiFi.begin(ssid, pass);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nlistener IP address: ");
  Serial.println(udpServer);
  udp.begin(UDP_PORT);
  Serial.print("listening on udp port: ");
  Serial.println(UDP_PORT);
}


void loop() {

  if (udp.parsePacket()) {
    //new packet..
    int recv = udp.read(buff, sizeof(buff));
    if (recv == sizeof(UDPDatagram)) {
      memcpy(&udpDatagram,&buff, sizeof(UDPDatagram));
      if (crcVerify()) {
        Serial.println("Packet received..");
      } else{
        Serial.println("Ignoring packet..");
      }
    }
  }
}

bool crcVerify() {
  uint16_t crc = 0;
  Serial.print("Datagram data ");
  for (int i = 0; i < sizeof(udpDatagram) - 2; i++) {
    Serial.print((uint8_t)((uint8_t *)&udpDatagram)[i], HEX);
    Serial.print(' ');
    crc += ((uint8_t *)&udpDatagram)[i];
  }
  return (crc == udpDatagram.crc);
}
