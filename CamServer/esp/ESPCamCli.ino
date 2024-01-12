#include <WiFi.h>
#include <esp_camera.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <lwip/sockets.h>
#include <IPAddress.h>
#include <Update.h>

#define RED_LED 2


/*
// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
*/

// Pin defs for CAMERA_MODEL_ESP32S3_EYE
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13



#define JPG_CHUNK_SIZE  1429 //vodo
#define OTA_CHUNK_SIZE  4096

//command byte..
#define CMD_NAK        0
#define CMD_ACK        1
#define CMD_SYNC       2
#define CMD_IDENT      3
#define CMD_JPG_BEGIN  4
#define CMD_JPG_CHUNK  5
#define CMD_JPG_END    6
#define CMD_PAUSE      7
#define CMD_CONFIG     8
#define CMD_OTA_BEGIN  9
#define CMD_OTA_CHUNK  10
#define CMD_OTA_END    11

//System states..
#define STATE_WIFI   0
#define STATE_IP     1
#define STATE_CLI    2
#define STATE_IDENT  3
#define STATE_SYNC   4
#define STATE_OTA    5
#define STATE_PAUSE  6
#define STATE_FRAME  7
#define STATE_CHUNK  8

// firmware version..
#define CAM_VER_LO 0
#define CAM_VER_HI 1

// Packet Identification.. ~q.. forever q..
#define IDENT_LO 113
#define IDENT_HI 126
//recv states
#define RECV_HEADER 0
#define RECV_EXTRA  1


//packet header..
struct __attribute__((__packed__)) PacketHdr {
  byte      Ident[2];
  byte      Command;
  uint32_t  DataSize;
};
//jpg begin packet..
struct __attribute__((__packed__)) PacketJpgBegin {
  uint32_t  imSize;
};

//ident packet
struct __attribute__((__packed__)) PacketIdent {
  byte      verLo;
  byte      verHi;
  byte      camNum;
};
//sync packet
struct __attribute__((__packed__)) PacketSync {
  uint16_t   year;
  byte      month;
  byte      day;
  byte      hour;
  byte      min;
  byte      sec;
};


//cam config..
camera_config_t config;
//remember things..
Preferences prefs;
//connection to server
WiFiClient client;

//System state..
byte stateSys = STATE_WIFI;


const char *ssid = "*****";
const char *password = "*******";

char *host = "192.168.0.10";  // IP of cam server
//char *host = "192.168.0.24";  // IP of cam server

//millis timer to limit cap rate..
unsigned long lastCap = 0;
unsigned long capDelay = 100ul;
int videoFlag = 1;

//generic millis timer, sys state machine..
unsigned long lastGo = 0;
unsigned long intervalGo = 15000ul;

//your webistie file containing current server ip..
char *myWebsiteFile ="http://www.yoursite.com/stuff/eggs.dat\n";
// ip address of server, stored using preferences..
char serverIP[20] ={'1','9','2','.','1','6','8','.','0','.','1','0','\n'};
//server port..
int port = 50001;              
//cam number stored in prefs..
byte camNum = 99;

//frame grabber vars..
camera_fb_t *fb = NULL;
int sentBytes = 0;
byte stateFrame = 0;
//incomeing buffer..
byte inBuff[OTA_CHUNK_SIZE];
//outgoing buffer..
byte outBuff[OTA_CHUNK_SIZE];
PacketHdr hdr;
PacketIdent ident;
PacketSync syncp;
PacketJpgBegin jpgBegin;

int recvCount = 0;
int sentCount = 0;
int imgSent = 0;
byte recvState = 0;
int32_t firmSize, firmRecvd;
int firmChunk = 0;
bool OTAbegun = false;
bool identified = false;
 
unsigned long lastBlink;
int intervalBlink = 1000;

bool syncSent = false;
bool identSent = false;

bool debugInfo = false;
int enable = 1;


void setup(void) {
  // serial for debug
  Serial.begin(115200);
  //set wifi mode, we'll connect later
  WiFi.mode(WIFI_STA);
  
  pinMode(RED_LED,OUTPUT);
  
  
  //start cam
  initCamera();
  
  
  prefs.begin("esp-cam", false);
  char tmpServerIP[20];
  prefs.getString("ServerIP",tmpServerIP,20);
  camNum = prefs.getUChar("CamNum",99);
  Serial.printf("Cam: %d Starting up...\n",camNum);
  //prefs.putString("ServerIP",serverIP,20);
  prefs.end();
  
    //validate..
  IPAddress ip;
  if (strlen(tmpServerIP)>0){
   if (ip.fromString(tmpServerIP)){
       strncpy(serverIP,tmpServerIP,sizeof(serverIP));                    
      } 
  }
}




void loop(void) {

  unsigned long now = millis();
  
  if (now-lastBlink>=intervalBlink){
    lastBlink = now;
    digitalWrite(RED_LED,!digitalRead(RED_LED));
  }

  if (WiFi.status() == WL_CONNECTED) {
      if (client.connected()) {
        checkIncoming();
      }
  }

//enter main system state mahcine..
  switch (stateSys) {
    case STATE_WIFI:  //connect wifi
      if (WiFi.status() != WL_CONNECTED) {
        if (now - lastGo >= intervalGo) {
          lastGo = now;
          WiFi.disconnect();
          WiFi.begin(ssid, password);
          Serial.println("Wfifi connecting..");
        }
      } else {
        stateSys = STATE_CLI;
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("RSSI:");
        Serial.println(WiFi.RSSI());
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("TX power:");
        Serial.println(WiFi.getTxPower());
      }
      break;
    case STATE_CLI:  //connect client
      if (now - lastGo >= intervalGo) {
        lastGo = now;
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connecting to server..");
          Serial.print("RSSI:");
          Serial.println(WiFi.RSSI());
          
          if (client.connect(host, port)) {
            Serial.println("Connected to cam server.");
             client.setNoDelay(true);
             client.setSocketOption(IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
            identified = false;
            stateSys = STATE_IDENT;//identify..
            //get ready for frame grabbing
            sentBytes = 0;
            stateFrame = 0;
            if (!fb == NULL) {esp_camera_fb_return(fb); fb = NULL;}
          } else Serial.println("Connection attempt failed..");
        } else{
           stateSys = STATE_WIFI;
           lastGo = now;
        }
      }
      break;
    case STATE_IDENT:  //indentify
      if (now - lastGo >= intervalGo) {
        lastGo = now;
      if (!client.connected()) {
       stateSys = STATE_CLI;
      } else
       {
        if (!identified){ 
         if (debugInfo){ 
         Serial.println("Sending Ident..");
         Serial.print("cam num:");
         Serial.println(camNum);
         }
         //send ident packet to server..
         hdr.Ident[0] = IDENT_LO;
         hdr.Ident[1] = IDENT_HI; 
         hdr.Command  = CMD_IDENT;
         hdr.DataSize = sizeof(PacketIdent);
         ident.camNum = camNum;
         ident.verHi  = CAM_VER_HI;
         ident.verLo  = CAM_VER_LO;
         sentCount = sizeof(PacketHdr);
         memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
         memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&ident,sizeof(PacketIdent));
         sentCount+=sizeof(PacketIdent);
         sentCount = client.write(outBuff,sentCount);
         if (debugInfo){
         Serial.print("bytes sent:");
         Serial.println(sentCount);
         }
        }
       }
      }
      break;
    case STATE_SYNC:  //sync time with server..
      if (now - lastGo >= intervalGo) {
        lastGo = now;
      if (!client.connected()) {
       stateSys = STATE_CLI;
      } else{
       if (debugInfo) Serial.print("Requesting Sync..:"); 
       hdr.Ident[0] = IDENT_LO;
       hdr.Ident[1] = IDENT_HI; 
       hdr.Command  = CMD_SYNC;
       hdr.DataSize = 0;
       sentCount = client.write((uint8_t *)&hdr,  sizeof(PacketHdr));  
       if (debugInfo) Serial.print(sentCount);              
      }
      }
      break;
    case STATE_OTA:  //updating software..
      if (now - lastGo >= intervalGo) {
        lastGo = now;
      if (!client.connected()) {
       stateSys = STATE_CLI;
       OTAbegun = false;
       Update.abort();       
      }
      }
      break;
    case STATE_PAUSE:  //do nothing..
      if (now - lastGo >= intervalGo) {
        lastGo = now;
      if (!client.connected()) {
       stateSys = STATE_CLI;
      }
      }
      break;
    case STATE_FRAME:  //start sending a jpg
      if (now - lastGo >= intervalGo) {
      if (client.connected()) {
        //start a frame
      if ( beginFrame() ){
       if(debugInfo) Serial.println("Image begun..");
        stateSys = STATE_PAUSE;} else lastGo = now;
        
      } else stateSys = STATE_CLI;
      }
      break;
    case STATE_CHUNK:  //send a frame chunk to server..
      if (client.connected()) {
        //send frame
         if ( chunkFrame()) {if (debugInfo)Serial.println("Image sent..");} 
        stateSys = STATE_PAUSE;
      } else stateSys = STATE_CLI;
      break;
      
  }
  yield();
}


//check for incoming packets
bool checkIncoming() {
  bool result = false;
  int extraRecvd = 0;
 if (recvState == RECV_HEADER){ 
  //check for incoming packet
  if (client.available() >= sizeof(PacketHdr)) {
    //have a packet
    if (client.read(inBuff, sizeof(PacketHdr)) == sizeof(PacketHdr)) {
      //got a header..
      if (inBuff[0] == IDENT_LO && inBuff[1] == IDENT_HI) {
        //matches our ident, copy buff to structure..
        memcpy(&hdr, &inBuff, sizeof(PacketHdr));
        //check for extra data..
        if (hdr.DataSize > 0 && hdr.DataSize < 512){
          //zero buff
          memset(inBuff, 0, sizeof(inBuff));
          extraRecvd = client.read(inBuff, hdr.DataSize);
          if (extraRecvd < 0) extraRecvd = 0;
         if ( extraRecvd == hdr.DataSize) {
          result = true;
          //leave eaxtra data in buffer for later processing..
         } else {
          //didn't get it all..
           recvState = RECV_EXTRA;
           recvCount = extraRecvd;          
         }
        } else if (hdr.DataSize == 0){
         result = true; //just a header
        } else {
           //need more
           recvState = RECV_EXTRA;
           recvCount = 0;
        }
          if (result && recvState == RECV_HEADER){ 
          //process incoming..
          processIncoming();  
      }
      //zero buff
      memset(inBuff, 0, sizeof(inBuff));
     } else Serial.println("Bad Ident..");  
    }
   }
  } else
    if (recvState == RECV_EXTRA){
     if (client.available() >= hdr.DataSize ) {
       //recv a chunk
          extraRecvd = client.read(inBuff, hdr.DataSize);
          recvCount+=extraRecvd;
         if ( recvCount == hdr.DataSize) {
          result = true;
          //leave eaxtra data in buffer for later processing..
          recvState = RECV_HEADER;
          recvCount = 0;
         }  
          if (result){ 
          //process incoming..
          processIncoming();  
          //zero buff
          memset(inBuff, 0, sizeof(inBuff));
           }
        }
    }
  return result;
}

//process incoming packet..
void processIncoming(){
  /*
#define CMD_NACK       0
#define CMD_ACK        1
#define CMD_SYNC       2
#define CMD_IDENT      3
#define CMD_JPG        4
#define CMD_PAUSE      5
#define CMD_CONFIG     6
#define CMD_OTA_BEGIN  7
#define CMD_OTA_CHUNK  8
#define CMD_OTA_END    9
  
  */
  if (debugInfo){
  Serial.print("Processing command byte:");
  Serial.println(hdr.Command);
  }
  switch (hdr.Command){
    case CMD_NAK:     break;
    case CMD_ACK:     break;
    case CMD_SYNC: if (hdr.DataSize == sizeof(PacketSync)){
                       memcpy((uint8_t *)&syncp,inBuff,sizeof(PacketSync));
                       SyncTime(syncp);
                       } 
                       break;
    case CMD_IDENT:if (hdr.DataSize == sizeof(PacketIdent)){
                       memcpy((uint8_t *)&ident,inBuff,sizeof(PacketIdent));
                       SetIdent(ident);
                       } 
                       break;
    case CMD_JPG_BEGIN : stateSys = STATE_FRAME;lastGo-=intervalGo;
                       break;
    case CMD_JPG_CHUNK : stateSys = STATE_CHUNK;lastGo-=intervalGo;
                       break;
    case CMD_PAUSE: stateSys = STATE_PAUSE;
                       break;
    case CMD_CONFIG:   break;
    case CMD_OTA_BEGIN: OTAbegin(); break;
    case CMD_OTA_CHUNK: OTAchunk(); break;
    case CMD_OTA_END  : OTAend();   break;
  }
  
  
}

void SyncTime(PacketSync sp){
struct tm tm;
    tm.tm_year = sp.year - 1900;
    tm.tm_mon  = sp.month - 1;//dec
    tm.tm_mday = sp.day;
    tm.tm_hour = sp.hour;
    tm.tm_min  = sp.min;
    tm.tm_sec  = sp.sec;
    time_t t   = mktime(&tm);
    if (debugInfo){
    Serial.printf("Syncing Time: %s\n", asctime(&tm));
    Serial.printf("Sync Packet: year:%d month:%d day:%d hour:%d min:%d",sp.year,sp.month,sp.day,sp.hour,sp.min);
    }
    struct timeval now = { .tv_sec = t };
    stateSys = STATE_FRAME;//start sending jpegs..            
    settimeofday(&now, NULL);  
    if (debugInfo) {Serial.print("\nSystem time now:");
    TimePrint();}
}

void TimePrint(){
  struct tm now;
  getLocalTime(&now,0);
  if (now.tm_year >= 117) Serial.println(&now, "%B %d %Y %H:%M:%S (%A)");
}


void SetIdent(PacketIdent idp){
  //set cam num..
if (idp.camNum != camNum){
  camNum = idp.camNum;
  if (debugInfo){
  Serial.print("Cam Number:");
  Serial.println(camNum);
  }
  prefs.begin("esp-cam", false);
  prefs.putUChar("CamNum",camNum);
  prefs.end();  
} else
  if (debugInfo) Serial.println("Ident accepted, no changes..");
  identified = true;
  stateSys = STATE_SYNC;
}


void OTAbegin(){
    int32_t value ;
    if (hdr.DataSize == sizeof(int32_t)){
       //get the firm size  
       memcpy((uint8_t *)&value,&inBuff,sizeof(int32_t));
       firmSize = value;
       firmChunk = 0;
       firmRecvd = 0;
       OTAbegun = true;
       stateSys = STATE_OTA;
      // Serial.print("Starting update, Firm size");
      // Serial.println(_firmSize);
       Update.begin(firmSize);
       if (debugInfo) Serial.println("Requesting chunk 1");
       //start asking for chunks
       hdr.Command = CMD_OTA_CHUNK;
       hdr.DataSize = 0;
       client.write((uint8_t *)&hdr, sizeof(PacketHdr));                 
    } else
      {
       //send nak back
       hdr.Command = CMD_NAK;
       client.write((uint8_t *)&hdr, sizeof(PacketHdr));     
      }
}

void OTAchunk(){
     //must have something, but can't have too much..  
    if (hdr.DataSize > 0 && hdr.DataSize <= OTA_CHUNK_SIZE && OTAbegun){
       firmChunk++;
       firmRecvd+=hdr.DataSize;
       Update.write( inBuff, hdr.DataSize);
      // Serial.println(_firmChunk);
       //ask for next chunk
       hdr.Command = CMD_OTA_CHUNK;
       hdr.DataSize = 0;
       client.write((uint8_t *)&hdr, sizeof(PacketHdr));
    } else
      {
        if (OTAbegun){
          OTAbegun = false;
          stateSys = STATE_IDENT;
          Update.abort();       
        }
       //send nak back
       hdr.Command = CMD_NAK;
       hdr.DataSize = 0;
       client.write((uint8_t *)&hdr, sizeof(PacketHdr));
      }
  
}

void OTAend(){
  if (firmRecvd == firmSize){
    //Serial.println("Update completed..");
   if ( Update.end(true)){
    ESP.restart();
   } else
     {
       //just send nak back
       hdr.Command = CMD_NAK;
       hdr.DataSize = 0;
       client.write((uint8_t *)&hdr, sizeof(PacketHdr));
       OTAbegun = false;      
       stateSys = STATE_IDENT;
     }
  } else
     { //abort update
       Update.abort();
       //send nak back
       hdr.Command = CMD_NAK;
       hdr.DataSize = 0;
       client.write((uint8_t *)&hdr, sizeof(PacketHdr));
       OTAbegun = false;     
       stateSys = STATE_IDENT;
 }
}

bool beginFrame() {
  bool result = false;
  int scnt = 0;
  //if frame buffer still lives, give it back..
  if (fb != NULL) {
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  fb = esp_camera_fb_get();
  if (fb != NULL) {
    hdr.Command = CMD_JPG_BEGIN;
    hdr.DataSize = sizeof(PacketJpgBegin);
    jpgBegin.imSize = fb->len;
    sentCount = sizeof(PacketHdr);
    memcpy(outBuff, (uint8_t *)&hdr, sizeof(PacketHdr));
    memcpy((uint8_t *)&outBuff[sentCount], (uint8_t *)&jpgBegin, sizeof(PacketJpgBegin));
    sentCount += sizeof(PacketJpgBegin);
    scnt = client.write(outBuff, sentCount);
    if (debugInfo){
    Serial.print("****** begin image size:");
    Serial.print(fb->len);
    Serial.print(" Sent request:");
    Serial.println(sentCount);
    }
    if (scnt == sentCount) result = true;
    sentBytes = 0;
  }
  return result;
}

bool chunkFrame(){
  bool result = false;
 if (fb->len-sentBytes>=JPG_CHUNK_SIZE){    
      if (((fb->len-sentBytes)-JPG_CHUNK_SIZE)<JPG_CHUNK_SIZE){
        //last packet can be a bit bigger..
         if (debugInfo){        
           Serial.print("Sending last jpg chunk:");
           Serial.println(fb->len-sentBytes);          
           Serial.print(" Total:");
           Serial.print(fb->len);
           }
           sentCount = sizeof(PacketHdr);
           hdr.Command = CMD_JPG_END;
           hdr.DataSize = fb->len - sentBytes;
           memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
           memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&fb->buf[sentBytes], fb->len - sentBytes);
           sentCount+=(fb->len - sentBytes);
           sentCount = client.write(outBuff,sentCount);   
          if(debugInfo){
           Serial.print(" sent: ");
           Serial.println(sentCount);
          }
           //remember what we send..
           if (sentCount>0){
              sentCount-=sizeof(PacketHdr);
              sentBytes+=sentCount;
           }
           //have we sent it all??   
           if (sentBytes == fb->len){
           esp_camera_fb_return(fb);            
            fb = NULL;
           stateFrame = 0;
           result = true;   
           }        
      } else{
          if (debugInfo){
           Serial.print("Sending jpg chunk:");
           Serial.print(JPG_CHUNK_SIZE);
           Serial.print(" Remaining:");
           Serial.print(fb->len-sentBytes);
          }
           sentCount = sizeof(PacketHdr);
           hdr.Command = CMD_JPG_CHUNK;
           hdr.DataSize = JPG_CHUNK_SIZE;
           memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
           memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&fb->buf[sentBytes], JPG_CHUNK_SIZE);
           sentCount+=JPG_CHUNK_SIZE;
           sentCount = client.write(outBuff,sentCount);   
           if (debugInfo){
           Serial.print(" sent: ");
           Serial.println(sentCount);
           }
           //remember what we send..
           if (sentCount>0){
              sentCount-=sizeof(PacketHdr);
              sentBytes+=sentCount;
           }
      }
     } else {
        if (fb->len - sentBytes >0){
          if (debugInfo){
           Serial.print("Sending last jpg chunk:");
           Serial.println(fb->len-sentBytes);          
           Serial.print(" Total:");
           Serial.print(fb->len);
          }
           sentCount = sizeof(PacketHdr);
           hdr.Command = CMD_JPG_END;
           hdr.DataSize = fb->len - sentBytes;
           memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
           memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&fb->buf[sentBytes], fb->len - sentBytes);
           sentCount+=fb->len - sentBytes;
           sentCount = client.write(outBuff,sentCount);   
           if (debugInfo){
           Serial.print(" sent: ");
           Serial.println(sentCount);
           }
           //remember what we send..
           if (sentCount>0){
              sentCount-=sizeof(PacketHdr);
              sentBytes+=sentCount;
           }
           //have we sent it all??   
           if (sentBytes == fb->len){
           esp_camera_fb_return(fb);
           fb = NULL;
           stateFrame = 0;
           result = true;   
           }
        } else {
          stateFrame = 0;
          result = (fb->len-sentBytes == 0);          
          esp_camera_fb_return(fb);
          fb = NULL;
         hdr.Command = CMD_JPG_END;
         hdr.DataSize = 0;
         client.write((uint8_t *)&hdr, sizeof(PacketHdr));
        }     
     }  
     
   return result;  
     
}



bool SendFrame() {
  bool result = false;
  imgSent = 0;
  switch (stateFrame){
     case 0: if (millis() - lastCap >= capDelay) {
              lastCap = millis();
              //if frame buffer still lives, give it back..
              if (fb != NULL){
                esp_camera_fb_return(fb);            
                 fb = NULL;                
              }
              fb = esp_camera_fb_get();
             if (fb != NULL) {
               hdr.Command = CMD_JPG_BEGIN;
               hdr.DataSize = sizeof(PacketJpgBegin);
               jpgBegin.imSize = fb->len;
               sentCount = sizeof(PacketHdr);
               memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
               memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&jpgBegin, sizeof(PacketJpgBegin));
               sentCount+=sizeof(PacketJpgBegin);
               sentCount = client.write(outBuff,sentCount);                  
               if (debugInfo){
               Serial.print("****** begin image size:");
               Serial.print(fb->len);
               Serial.print(" Sent request:");
               Serial.println(sentCount);
               }
               sentBytes = 0;
               stateFrame++;
             } else {
              Serial.println("*** failed to get image ****");
             }
           }
     break;
     case 1: if (fb->len-sentBytes>=JPG_CHUNK_SIZE){
      
      if (((fb->len-sentBytes)-JPG_CHUNK_SIZE)<JPG_CHUNK_SIZE){
        //last packet can be a bit bigger..
        if (debugInfo){
           Serial.print("Sending last jpg chunk:");
           Serial.println(fb->len-sentBytes);          
           Serial.print(" Total:");
           Serial.print(fb->len);
        }
           sentCount = sizeof(PacketHdr);
           hdr.Command = CMD_JPG_END;
           hdr.DataSize = fb->len - sentBytes;
           memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
           memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&fb->buf[sentBytes], fb->len - sentBytes);
           sentCount+=(fb->len - sentBytes);
           sentCount = client.write(outBuff,sentCount);   
           if (debugInfo){
           Serial.print(" sent: ");
           Serial.println(sentCount);
           }
           //remember what we send..
           if (sentCount>0){
              sentCount-=sizeof(PacketHdr);
              sentBytes+=sentCount;
           }
           //have we sent it all??   
           if (sentBytes == fb->len){
           esp_camera_fb_return(fb);            
            fb = NULL;
           stateFrame = 0;
           result = true;   
           }        
      } else{
         if(debugInfo){
           Serial.print("Sending jpg chunk:");
           Serial.print(JPG_CHUNK_SIZE);
           Serial.print(" Remaining:");
           Serial.print(fb->len-sentBytes);
           }
           sentCount = sizeof(PacketHdr);
           hdr.Command = CMD_JPG_CHUNK;
           hdr.DataSize = JPG_CHUNK_SIZE;
           memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
           memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&fb->buf[sentBytes], JPG_CHUNK_SIZE);
           sentCount+=JPG_CHUNK_SIZE;
           sentCount = client.write(outBuff,sentCount);   
           if (debugInfo){
           Serial.print(" sent: ");
           Serial.println(sentCount);
           }
           //remember what we send..
           if (sentCount>0){
              sentCount-=sizeof(PacketHdr);
              sentBytes+=sentCount;
           }
      }
     } else {
        if (fb->len - sentBytes >0){
          if (debugInfo){
           Serial.print("Sending last jpg chunk:");
           Serial.println(fb->len-sentBytes);          
           Serial.print(" Total:");
           Serial.print(fb->len);
          }
           sentCount = sizeof(PacketHdr);
           hdr.Command = CMD_JPG_END;
           hdr.DataSize = fb->len - sentBytes;
           memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
           memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&fb->buf[sentBytes], fb->len - sentBytes);
           sentCount+=fb->len - sentBytes;
           sentCount = client.write(outBuff,sentCount);   
          if (debugInfo){ 
           Serial.print(" sent: ");
           Serial.println(sentCount);
          }
           //remember what we send..
           if (sentCount>0){
              sentCount-=sizeof(PacketHdr);
              sentBytes+=sentCount;
           }
           //have we sent it all??   
           if (sentBytes == fb->len){
           esp_camera_fb_return(fb);
           fb = NULL;
           stateFrame = 0;
           result = true;   
           }
        } else {
          stateFrame = 0;
          result = (fb->len-sentBytes == 0);          
          esp_camera_fb_return(fb);
          fb = NULL;
         hdr.Command = CMD_JPG_END;
         hdr.DataSize = 0;
         client.write((uint8_t *)&hdr, sizeof(PacketHdr));
        }     
     }
     break;
  }
  return result;
}


bool initCamera() {
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;    
  if (psramFound()) {
    //if we have additional ram
    Serial.println("Using PSRAM..");
//    config.frame_size = FRAMESIZE_UXGA;
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    //no extra ram, lower res and 1 frame buffer
    Serial.println("PSRAM not found..");
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return false;
  }
  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  s->set_vflip(s, 1); // flip it back
  s->set_brightness(s, 1); // up the brightness just a bit
  s->set_saturation(s, 0); // lower the saturation
  
  
  return true;
}

//check the website for a wan ip address change..
bool CheckServerIP(){
  bool result = false;
  char newIP[20];
  HTTPClient http;
  http.begin(myWebsiteFile); 
  int httpCode = http.GET();
      if(httpCode > 0) {
           // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                if (payload.length()>0 && (payload.length() <  sizeof(newIP) )){
                  strncpy(newIP,payload.c_str(),sizeof(newIP));
                  //validate..
                  IPAddress ip;
                  if (ip.fromString(newIP)){
                    result = true;
                  strncpy(serverIP,newIP,sizeof(serverIP));                    
                  } else {
                    ScrambledEggs(newIP,sizeof(newIP),true);
                  if (ip.fromString(newIP)){
                    result = true;
                  strncpy(serverIP,newIP,sizeof(serverIP));                    
                  }    
                  }
                }
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();    
        return result;
}


void ScrambledEggs(char *ingrediants, int amount, bool eat)
{ if (amount >19) return;
    int i,j;
    char davinci[20];
    j = amount-2;
    char blender[20] = { 22, 53, 44, 71, 66, 177, 253, 122, 110, 124, 132, 33, 54, 72, 101,11,14,15,23,21 };
    for(i = 0; i < amount-1; i++){
      if (!eat)
        davinci[j] = ingrediants[i] ^ blender[i]; else
          davinci[i]=ingrediants[j] ^ blender[i];
        j--;
    }
    memcpy(ingrediants,davinci,amount-1);
}



