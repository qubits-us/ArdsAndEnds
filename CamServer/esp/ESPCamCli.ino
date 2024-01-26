#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_camera.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <lwip/sockets.h>
#include <IPAddress.h>
#include <Update.h>
#include <AsyncMorse.h>

//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

#if defined(CAMERA_MODEL_AI_THINKER) || defined(CAMERA_MODEL_ESP32S3_EYE)
#if defined(CAMERA_MODEL_AI_THINKER)
#define OB_LED 33
#endif
#if defined(CAMERA_MODEL_ESP32S3_EYE)
#define OB_LED 2
#endif
#elif 
#define OB_LED -1
#endif

#define FLASH_PIN 4 //ai thinker..

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
#define CMD_CONFIG_GET 8
#define CMD_CONFIG_SET 9
#define CMD_REC_ON     10
#define CMD_REC_OFF    11
#define CMD_ALERT_ON   12
#define CMD_ALERT_OFF  13
#define CMD_OTA_BEGIN  14
#define CMD_OTA_CHUNK  15
#define CMD_OTA_END    16

//System states..
#define STATE_SOS    0
#define STATE_WIFI   1
#define STATE_IP     2
#define STATE_CLI    3
#define STATE_IDENT  4
#define STATE_SYNC   5
#define STATE_OTA    6
#define STATE_PAUSE  7
#define STATE_FRAME  8
#define STATE_CHUNK  9

#define DISCV_START 0
#define DISCV_CHECK 1

// firmware version..
#define CAM_VER_LO 0
#define CAM_VER_HI 1

// Packet Identification.. ~q.. forever q..
#define IDENT_LO 113
#define IDENT_HI 126
//recv states
#define RECV_HEADER 0
#define RECV_EXTRA  1

#define UDP_PORT   50000


//discovery packet..
struct __attribute__((__packed__)) PacketDiscv {
  byte      Ident[2];
  uint16_t  Port;
  byte      Ip[20];
};

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
  byte      camId[40];
  byte      camNum;
  byte      verLo;
  byte      verHi;
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

//sensor packet
struct __attribute__((__packed__)) PacketSensor {
  int32_t  framesize;//0 - 10 -4bytes
  bool     scale;
  bool     binning;
  uint8_t  quality;//0 - 63
  int8_t   brightness;//-2 - 2
  int8_t   contrast;//-2 - 2
  int8_t   saturation;//-2 - 2
  int8_t   sharpness;//-2 - 2
  uint8_t  denoise;
  uint8_t  special_effect;//0 - 6
  uint8_t  wb_mode;//0 - 4
  uint8_t  awb;
  uint8_t  awb_gain;
  uint8_t  aec;
  uint8_t  aec2;
  int8_t   ae_level;//-2 - 2
  uint16_t aec_value;//0 - 1200
  uint8_t  agc;
  uint8_t  agc_gain;//0 - 30
  uint8_t  gainceiling;//0 - 6
  uint8_t  bpc;
  uint8_t  wpc;
  uint8_t  raw_gma;
  uint8_t  lenc;
  uint8_t  hmirror;
  uint8_t  vflip;
  uint8_t  dcw;
  uint8_t  colorbar;
  uint32_t capInterval;
  bool     debug;
};




//cam config..
camera_config_t config;
//remember things..
Preferences prefs;
//connection to server
WiFiClient client;
//listens for discovery packets..
WiFiUDP udp;

//System state..
byte stateSys = STATE_WIFI;
//Discovery state
byte stateDiscv = DISCV_START;

char ssid[20];
char password[20];
int knownAPs = 0;

//millis timer to limit cap rate..
unsigned long lastCap = 0;
unsigned long capDelay = 100ul;
int videoFlag = 1;

//generic millis timer, sys state machine..
unsigned long lastGo = 0;
unsigned long intervalGo = 15000ul;

//your webistie file containing current server ip..
char myWebsiteFile[200];
// ip address of server, stored using preferences..
char serverIP[20] ={'1','9','2','.','1','6','8','.','0','.','9','9','\n'};
//server port..
int port = 50001;              
//camnumber stored in prefs..
byte camNum = 99;
//empty guid..
byte camId[40] = {'{','0','0','0','0','0','0','0','0','-','0','0','0','0','-','0','0','0','0','-','0','0','0','0','-','0','0','0','0','0','0','0','0','0','0','0','0','}',0,0};

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
PacketSensor sensorPack;
PacketDiscv discvPack;

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

bool debugInfo = true;
int enable = 1;

int noActivity = 0;
int badConnCnt = 0;

//sos - serial config needed..
char serInBuff[80];
int  serInCnt = 0;
byte serState = 0;

AsyncMorse morse(OB_LED);




void setup(void) {
  // serial for debug
  Serial.begin(115200);
  //set wifi mode, we'll connect later
  WiFi.mode(WIFI_STA);
  if (OB_LED > 0){
  pinMode(OB_LED,OUTPUT);
  }
  
  client.setNoDelay(true);
  client.setSocketOption(IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
  
  
  //start cam
  initCamera();
  
  ident.verHi = CAM_VER_HI;
  ident.verLo = CAM_VER_LO;
  
  prefs.begin("esp-cam", false);
  char tmpBytes[40];
  int num;
  IPAddress ip;
  
  num = prefs.getBytesLength("ssid");
  if (num> 0 && num < 21){
  prefs.getBytes("ssid",tmpBytes,20);
    //validate..
  if (strlen(tmpBytes)>0){
       strncpy(ssid,tmpBytes,sizeof(ssid));  
       knownAPs = 1;   
       if (debugInfo){
        Serial.print("loaded ssid: ");
        Serial.println(ssid);
       }               
   }
}

  num = prefs.getBytesLength("password");
  if (num> 0 && num < 21){
  prefs.getBytes("password",tmpBytes,20);
    //validate..
  if (strlen(tmpBytes)>0){
       strncpy(password,tmpBytes,sizeof(password));     
       if (debugInfo){
        Serial.print("loaded pass: ");
        Serial.println(password);
       }               
    }
}


  
  num = prefs.getBytesLength("ServerIP");
  if (num> 0 && num < 21){
  prefs.getBytes("ServerIP",tmpBytes,20);
    //validate..
  if (strlen(tmpBytes)>0){
   if (ip.fromString(tmpBytes)){
       strncpy(serverIP,tmpBytes,sizeof(serverIP));     
       if (debugInfo){
        Serial.print("loaded server ip: ");
        Serial.println(serverIP);
       }               
      } 
  }
}


  camNum = prefs.getUChar("CamNum",99);
  
  unsigned int a = prefs.getUInt("CapInterval",100);
  if (capDelay != a) capDelay = a;
  
  num = prefs.getBytesLength("CamId");
  if (num> 0 && num < 41){
  prefs.getBytes("CamId",tmpBytes,40);
  memcpy(camId,tmpBytes,sizeof(camId));
}
  
  prefs.end();
  //copy to ident pack
  memcpy(ident.camId,camId,sizeof(camId));
  
  Serial.printf("Cam: %d Starting up...\n",camNum);
  for (int i = 0; i< sizeof(ident.camId);i++){
    Serial.print((char)ident.camId[i]);
  }
  Serial.println("");
  
  if (knownAPs > 0) stateSys = STATE_WIFI; else stateSys = STATE_SOS;
  
  
}




void loop(void) {

  unsigned long now = millis();
  
 if (OB_LED > 0 && stateSys != STATE_SOS) {
  if (now - lastBlink >= intervalBlink) {
    lastBlink = now;
    digitalWrite(OB_LED, !digitalRead(OB_LED));
    if (WiFi.status() == WL_CONNECTED && client.connected()) {
      noActivity++;
      if (noActivity > 99) {
        client.stop();
        Serial.println("No activity for 100 seconds..");
        Serial.print("Heap avail: ");
        Serial.println(ESP.getFreeHeap());
        Serial.print("Psram avail: ");
        Serial.println(ESP.getFreePsram());
        noActivity = 0;
      }
    }
  }
}

  if (WiFi.status() == WL_CONNECTED) {
      if (client.connected()) {
        checkIncoming();
      }
  }

//enter main system state mahcine..
  switch (stateSys) {

    case STATE_SOS:  //need some love.
      if (CheckSerial()){
        stateSys = STATE_WIFI;
        badConnCnt = 0;
      }
      break;    
    case STATE_WIFI:  //connect wifi
      if (WiFi.status() != WL_CONNECTED) {
        if (now - lastGo >= intervalGo) {
          intervalGo = 15000ul;
          lastGo = now;
          noActivity = 0;
          badConnCnt++;
          WiFi.disconnect();
          WiFi.begin(ssid, password);
          Serial.println("Wfifi connecting..");
          //not a known and bad 10 tries, sos again
          if (badConnCnt>10 && knownAPs == 0){
            serState = 0;
            stateSys = STATE_SOS;
          } else{
          WiFi.begin(ssid, password);
          Serial.println("Wfifi connecting..");            
          }
        }
      } else {
        stateSys = STATE_IP;
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("RSSI:");
        Serial.println(WiFi.RSSI());
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("TX power:");
        Serial.println(WiFi.getTxPower());
        //save it..
        if (knownAPs==0){
          knownAPs = 1;
           prefs.begin("esp-cam", false);
           prefs.putBytes("ssid",ssid,sizeof(ssid));
           prefs.putBytes("password",password,sizeof(password));
           prefs.end();
        }
        
        intervalGo = 2000ul;
      }
      break;
    case STATE_IP:  //get server ip..
      if (now - lastGo >= intervalGo) {
        intervalGo = 15000ul;
        lastGo = now;        
      if (WiFi.status() == WL_CONNECTED) {
        if (debugInfo){
          Serial.println("..IP Discovery..");
        }
        switch (stateDiscv){
            case DISCV_START: if (StartDiscovery()){stateDiscv = DISCV_CHECK;} break;
            case DISCV_CHECK: if (CheckDiscovery()){stateDiscv = DISCV_START; stateSys = STATE_CLI;
              if (debugInfo){
                Serial.print("Discovery completed New IP: ");
                Serial.println(serverIP);
                
              }
              } break;
        } 
      
      } else
      {
       stateSys = STATE_WIFI;//reconnect wifi
      }
      }
      break;
    case STATE_CLI:  //connect client
      if (now - lastGo >= intervalGo) {
        lastGo = now;
        intervalGo = 15000ul;
        noActivity = 0;
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connecting to server..");
          Serial.print("RSSI:");
          Serial.println(WiFi.RSSI());
          
          if (client.connect(serverIP, port)) {
            Serial.println("Connected to cam server.");
             client.setNoDelay(true);
             client.setSocketOption(IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
            identified = false;
            stateSys = STATE_IDENT;//identify..
            sentBytes = 0;
            stateFrame = 0;
            intervalGo = 2000ul;
            if (fb) {esp_camera_fb_return(fb); fb = NULL;}
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
        intervalGo = 30000ul;
        noActivity = 0;
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
         memcpy(ident.camId,camId,sizeof(camId));
         ident.verHi  = CAM_VER_HI;
         ident.verLo  = CAM_VER_LO;
         sentCount = sizeof(PacketHdr);
         memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
         memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&ident,sizeof(PacketIdent));
         sentCount+=sizeof(PacketIdent);
         sentCount = client.write(outBuff,sentCount);
         client.flush();
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
        intervalGo = 30000ul;
      if (!client.connected()) {
       stateSys = STATE_CLI;
      } else{
       if (debugInfo) Serial.print("Requesting Sync..:"); 
       hdr.Ident[0] = IDENT_LO;
       hdr.Ident[1] = IDENT_HI; 
       hdr.Command  = CMD_SYNC;
       hdr.DataSize = 0;
       sentCount = client.write((uint8_t *)&hdr,  sizeof(PacketHdr));  
       client.flush();
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
        intervalGo = 15000ul;
        lastGo = now;
        if (debugInfo){
          Serial.println("..Paused..");
        }
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
         if ( chunkFrame()) {if (debugInfo)Serial.println("Image sent..");
        stateSys = STATE_PAUSE;}
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
    noActivity = 0;
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
      noActivity = 0;
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
    case CMD_CONFIG_GET: GetConfig(); break;
    case CMD_CONFIG_SET:if (hdr.DataSize == sizeof(PacketSensor)){
                       memcpy((uint8_t *)&sensorPack,inBuff,sizeof(PacketSensor));
                       SetConfig(sensorPack);
                       }  break;
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
    struct timeval now = { .tv_sec = t,
                           .tv_usec = 0 };
    if (stateSys == STATE_SYNC)                       
    stateSys = STATE_FRAME;//start sending jpegs..   
    intervalGo = 2000ul;         
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
 if (memcmp(idp.camId,camId,sizeof(camId))!=0){
  //got a new id..
  memcpy(camId,idp.camId,sizeof(camId));
  //use incoming cam num
  camNum = idp.camNum;
  prefs.begin("esp-cam", false);
  prefs.putBytes("CamId", camId, sizeof(camId));
  prefs.putUChar("CamNum",camNum);
  prefs.end();
 } 
   else
  if (debugInfo) Serial.println("Ident accepted, no changes..");
  
  identified = true;
  stateSys = STATE_SYNC;
  intervalGo = 2000ul;
}

void SetConfig(PacketSensor ps){
  //set incoming config..
    sensor_t * s = esp_camera_sensor_get();
  if (s){ 
      if( ps.framesize != s->status.framesize){
        s->set_framesize(s, (framesize_t)ps.framesize);
      }
      
     if ( ps.scale != s->status.scale){
      s->status.scale = ps.scale;
     }
     
     if ( ps.binning != s->status.binning){
      s->status.binning = ps.binning;
     }
     
     if ( ps.quality != s->status.quality){
      s->set_quality(s, ps.quality);
     }
     
     if (ps.brightness != s->status.brightness){
      s->set_brightness(s,ps.brightness);
     }
     
     if ( ps.contrast != s->status.contrast){
      s->set_contrast(s,ps.contrast);
     }
     
     if (ps.saturation != s->status.saturation){
      s->set_saturation(s,ps.saturation);
     }
     
     if (ps.sharpness != s->status.sharpness){
      s->set_sharpness(s,ps.sharpness);
     }
     
     if (ps.denoise != s->status.denoise){
      s->set_denoise(s,ps.denoise);
     }
     
     if (ps.special_effect != s->status.special_effect){
      s->set_special_effect(s,ps.special_effect);
     }
     
     if (ps.wb_mode != s->status.wb_mode){
      s->set_wb_mode(s,ps.wb_mode);
     }
     
     if ( ps.awb != s->status.awb){
      s->set_whitebal(s,ps.awb);
     }
     
     if (ps.awb_gain != s->status.awb_gain){
      s->set_awb_gain(s,ps.awb_gain);
     }
     
     if (ps.aec != s->status.aec){
      s->set_exposure_ctrl(s,ps.aec);
     }
     
     if (ps.aec2 != s->status.aec2){
      s->set_aec2(s,ps.aec2);
     }
     
     if (ps.ae_level != s->status.ae_level){
      s->set_ae_level(s,ps.ae_level);
     }
     
     if (ps.aec_value != s->status.aec_value){
      s->set_aec_value(s,ps.aec_value);
     }
     
     if (ps.agc != s->status.agc){
      s->set_gain_ctrl(s,ps.agc);
     }
     
     if (ps.agc_gain != s->status.agc_gain){
      s->set_agc_gain(s,ps.agc_gain);
     }
     
     if (ps.gainceiling != s->status.gainceiling){
      s->set_gainceiling(s,(gainceiling_t)ps.gainceiling);
     }
     
     if (ps.bpc != s->status.bpc){
      s->set_bpc(s,ps.bpc);
     }
     
     if (ps.wpc != s->status.wpc){
      s->set_wpc(s,ps.wpc);
     }
     
     if (ps.raw_gma != s->status.raw_gma){
      s->set_raw_gma(s,ps.raw_gma);
     }
     
     if (ps.lenc != s->status.lenc){
      s->set_lenc(s,ps.lenc);
     }
     
     if (ps.hmirror != s->status.hmirror){
      s->set_hmirror(s,ps.hmirror);
     }
     
     if (ps.vflip != s->status.vflip){
      s->set_vflip(s,ps.vflip);
     }
     
     if (ps.dcw != s->status.dcw){
      s->set_dcw(s,ps.dcw);
     }
     
     if (ps.colorbar != s->status.colorbar){
      s->set_colorbar(s,ps.colorbar);
     }
     if (ps.debug != debugInfo){
      debugInfo = ps.debug;
     }
     if (ps.capInterval != capDelay){
      capDelay = ps.capInterval;
     }
  }  
}

void GetConfig(){
  //send the sensor..
    sensor_t * s = esp_camera_sensor_get();
  if (s){  
     sensorPack.framesize      = s->status.framesize;
     sensorPack.scale          = s->status.scale;
     sensorPack.binning        = s->status.binning;
     sensorPack.quality        = s->status.quality;
     sensorPack.brightness     = s->status.brightness;
     sensorPack.contrast       = s->status.contrast;
     sensorPack.saturation     = s->status.saturation;
     sensorPack.sharpness      = s->status.sharpness;
     sensorPack.denoise        = s->status.denoise;
     sensorPack.special_effect = s->status.special_effect;
     sensorPack.wb_mode        = s->status.wb_mode;
     sensorPack.awb            = s->status.awb;
     sensorPack.awb_gain       = s->status.awb_gain;
     sensorPack.aec            = s->status.aec;
     sensorPack.aec2           = s->status.aec2;
     sensorPack.ae_level       = s->status.ae_level;
     sensorPack.aec_value      = s->status.aec_value;
     sensorPack.agc            = s->status.agc;
     sensorPack.agc_gain       = s->status.agc_gain;
     sensorPack.gainceiling    = s->status.gainceiling;
     sensorPack.bpc            = s->status.bpc;
     sensorPack.wpc            = s->status.wpc;
     sensorPack.raw_gma        = s->status.raw_gma;
     sensorPack.lenc           = s->status.lenc;
     sensorPack.hmirror        = s->status.hmirror;
     sensorPack.vflip          = s->status.vflip;
     sensorPack.dcw            = s->status.dcw;
     sensorPack.colorbar       = s->status.colorbar;
     sensorPack.debug          = debugInfo;
     sensorPack.capInterval    = capDelay;
         //send ident packet to server..
         hdr.Ident[0] = IDENT_LO;
         hdr.Ident[1] = IDENT_HI; 
         hdr.Command  = CMD_CONFIG_SET;
         hdr.DataSize = sizeof(PacketSensor);
         sentCount = sizeof(PacketHdr);
         memcpy(outBuff,(uint8_t *)&hdr,sizeof(PacketHdr));
         memcpy((uint8_t *)&outBuff[sentCount],(uint8_t *)&sensorPack,sizeof(PacketSensor));
         sentCount+=sizeof(PacketSensor);
         sentCount = client.write(outBuff,sentCount);
         client.flush();
         if (debugInfo){
         Serial.print("bytes sent:");
         Serial.println(sentCount);
         }
     
     
     
  } else Serial.println("Error: failed to get sensor config..");
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
  if (fb != NULL ) {
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  fb = esp_camera_fb_get();
  if (fb != NULL ) {
    hdr.Command = CMD_JPG_BEGIN;
    hdr.DataSize = sizeof(PacketJpgBegin);
    jpgBegin.imSize = fb->len;
    sentCount = sizeof(PacketHdr);
    memcpy(outBuff, (uint8_t *)&hdr, sizeof(PacketHdr));
    memcpy((uint8_t *)&outBuff[sentCount], (uint8_t *)&jpgBegin, sizeof(PacketJpgBegin));
    sentCount += sizeof(PacketJpgBegin);
    scnt = client.write(outBuff, sentCount);
    client.flush();
    if (debugInfo){
    Serial.print("****** begin image size:");
    Serial.print(fb->len);
    Serial.print(" Sent request:");
    Serial.println(sentCount);
    }
    if (scnt == sentCount) result = true;
    sentBytes = 0;
  } else {
    if (debugInfo){
      Serial.println("..failed to get frame buffer..");
    }
  }
  return result;
}

bool chunkFrame(){
  bool result = false;
 if (fb != NULL){ 
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
           client.flush(); 
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
           client.flush();
           if (debugInfo){
           Serial.print(" sent: ");
           Serial.println(sentCount);
           }
           //remember what we send..
           if (sentCount>0){
              sentCount-=sizeof(PacketHdr);
              sentBytes+=sentCount;
              stateSys = STATE_PAUSE;
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
           client.flush();
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
          if (debugInfo){
           Serial.print("Sending last jpg chunk:");
           Serial.println(fb->len-sentBytes);          
           Serial.print(" Total:");
           Serial.print(fb->len);
          }          
          stateFrame = 0;
          result = (fb->len-sentBytes == 0);          
          esp_camera_fb_return(fb);
          fb = NULL;
         hdr.Command = CMD_JPG_END;
         hdr.DataSize = 0;
         sentCount = client.write((uint8_t *)&hdr, sizeof(PacketHdr));
         client.flush();
           if (debugInfo){
           Serial.print(" sent: ");
           Serial.println(sentCount);
           }
        }     
     } 
 } else {
    stateSys = STATE_PAUSE;
    Serial.println("invalid frame buffer..");
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
//  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
//  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
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
 // s->set_vflip(s, 1); // flip it back
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

bool StartDiscovery(){
  bool result = false;
  if (udp.begin(UDP_PORT)){
 if (debugInfo) {  
  Serial.print("listening on udp port: ");
  Serial.println(UDP_PORT);
 }
  result = true;
  }
  return result;
}

bool CheckDiscovery() {
  bool result = false;
  //char newIP[20];
  if (udp.parsePacket()) {
    //new packet..
    memset(inBuff,0,sizeof(inBuff));//empty..
    int recv = udp.read(inBuff, sizeof(inBuff));//fill
    if (recv > 0) {
      if (recv == sizeof(discvPack)) {
      if (inBuff[0] == IDENT_LO && inBuff[1] == IDENT_HI) {
        memcpy((uint8_t *)&discvPack, inBuff, recv);
        //validate ip..
        IPAddress ip;
             prefs.begin("esp-cam", false);
        if (ip.fromString((const char*)&discvPack.Ip)) {
          result = true;
          
         if (memcmp(serverIP,discvPack.Ip,sizeof(serverIP)) != 0){ 
          strncpy(serverIP, (const char*)&discvPack.Ip, sizeof(serverIP));
             prefs.putBytes("ServerIP", serverIP, sizeof(serverIP));
         }
          port = discvPack.Port;
        } else {
          ScrambledEggs((char*)&discvPack.Ip, sizeof(discvPack.Ip), true);
          if (ip.fromString((const char*)&discvPack.Ip)) {
            result = true;
         if (memcmp(serverIP,discvPack.Ip,sizeof(serverIP)) != 0){ 
            strncpy(serverIP, (const char*)&discvPack.Ip, sizeof(serverIP));
             prefs.putBytes("ServerIP", serverIP, sizeof(serverIP));
         }
            port = discvPack.Port;
          }
        }
        prefs.end();
       } else {
          if (debugInfo){
            Serial.println("Bad Ident");
          }
       }
      } else {
        if (debugInfo){
          Serial.print("Invalid size, expecting: ");
          Serial.print(sizeof(discvPack));
          Serial.print(" received: ");
          Serial.println(recv);
        }
      }
    } else {
        if (debugInfo){
          Serial.println("nothing received..");
        }
    }
  } else {
      if (debugInfo){
        Serial.println("no packets..");
      }
  }
  return result;
}

bool StopDiscovery(){
 bool result = false; 
  udp.stop();
  result = true;
 return result; 
}


bool CheckSerial(){
  bool result = false;
  if (serState == 0){
    Serial.println("Enter network SSID:");
    morse.MorseString("SOS",true);
    serState = 1;
  }
  if (Serial.available()){
    char c = Serial.read();
    if (c != '\n' && c != 13 && isalnum(c)){
      serInBuff[serInCnt]= c;
      serInCnt++;
      if (serInCnt>sizeof(serInBuff)-2){
        //potential overflow.. wrap it for now
        serInCnt = 0;
      }
    } else
      if (c == '\n'){
        if (serState == 1){
          if (serInCnt > 0){
           serInBuff[serInCnt]=0;//terminate
           memcpy(ssid,serInBuff,serInCnt);//got something..
           serState = 2;
           serInCnt = 0;
           Serial.print("Enter network key for ");
           Serial.print(ssid);Serial.println(":");
         } 
       } else
       if (serState == 2){
          if (serInCnt > 0){
           serInBuff[serInCnt]=0;//terminate
           memcpy(password,serInBuff,serInCnt);//got something..
          } else memset(password,0,sizeof(password));//erase
           serState = 0;
           serInCnt = 0;
           Serial.print("New credentials accepted ");
           Serial.print(ssid);Serial.print(":");
           Serial.println(password);
           morse.MorseStop();
           result = true;
       } 
      }
  }
  //pump our sos..
  morse.MorseLoop();
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



