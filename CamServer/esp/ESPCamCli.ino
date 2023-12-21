#include <WiFi.h>
#include <esp_camera.h>

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


#define STATE_WIFI  0
#define STATE_CLI  1
#define STATE_FRAME  2

camera_config_t config;


WiFiClient client;

byte stateSys = STATE_WIFI;


const char *ssid = "*****";
const char *password = "******";

char *host = "192.168.0.10";  // IP of cam server
int port = 50001;              // server port
//cam server vars
unsigned long lastCap = 0;
int capDelay = 100;
int videoFlag = 1;

unsigned long lastConn = 0;
unsigned long intervalReconn = 15000ul;



void setup(void) {
  // serial for debug
  Serial.begin(9600);
  //set wifi mode, we'll connect later
  WiFi.mode(WIFI_STA);
  //start cam
  initCamera();
}




void loop(void) {

  unsigned long now = millis();

  switch (stateSys) {
    case STATE_WIFI:  //connect wifi
      if (WiFi.status() != WL_CONNECTED) {
        if (now - lastConn >= intervalReconn) {
          lastConn = now;
          WiFi.disconnect();
          WiFi.begin(ssid, password);
          Serial.println("Wfifi connecting..");
        }
      } else {
        stateSys++;
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
      }
      break;
    case STATE_CLI:  //connect client
      if (now - lastConn >= intervalReconn) {
        lastConn = now;
        if (WiFi.status() == WL_CONNECTED) {
          if (client.connect(host, port)) {
            Serial.println("Connected to cam server.");
            stateSys++;
          }
        } else{
           stateSys--;
           lastConn = now;
        }
      }
      break;
    case STATE_FRAME:  //send a frame to server..
      if (now - lastConn >= intervalReconn) {
      if (client.connected()) {
        //send frame
          SendFrame();
      } else stateSys--;
      }
      break;
  }
}

void SendFrame() {

      camera_fb_t *fb = NULL;
      if (millis() - lastCap >= capDelay) {
        lastCap = millis();
        fb = esp_camera_fb_get();
        if (fb != NULL) {
          uint8_t imgLen[4];
          imgLen[0] = fb->len >> 0;
          imgLen[1] = fb->len >> 8;
          imgLen[2] = fb->len >> 16;
          imgLen[3] = fb->len >> 24;
          client.write(imgLen, 4);
           Serial.print("Camera sending:");
            Serial.println(fb->len);
          client.write(fb->buf, fb->len);
          esp_camera_fb_return(fb);
        } else
          Serial.println("Capture failed.");
      }
    }


void initCamera() {
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
  if (psramFound()) {
    //if we have additional ram
    Serial.println("Using PSRAM..");
//    config.frame_size = FRAMESIZE_UXGA;
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    //no extra ram, lower res and 1 frame buffer
    Serial.println("PSRAM not found..");
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
