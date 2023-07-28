#include <U8glib.h>

//rotary encoder
#define CLK2 2
#define DT2 3

//vars used in interupt declare volatile
volatile int counter = 20;  //keep track of steps
volatile int CLKstate;
volatile int lastCLKstate;
//display
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  //set this to whatever your using

int ballX = 64;
int ballY = 32;
int ballDir = 0;
byte ballVdir = 0;//0=up 1=dwn
byte ballAngle = 1;
int ballSpeed = 3;
byte paddleWidth = 24; //divisable by 3

void draw(void) {
  noInterrupts();
  int paddleY = counter;
  interrupts();


  u8g.drawBox(8, paddleY, 5, paddleWidth);   //player
  u8g.drawBox(ballX, ballY, 5, 5);  //the ball
}

void setup(void) {
  pinMode(CLK2, INPUT_PULLUP);
  pinMode(DT2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLK2), updateEncoder, CHANGE);
}  

void moveBall() {
  if (ballVdir == 0) {
    //up
    ballY += ballAngle;
    if (ballY >= 50) ballVdir = 1;
  } else {
    //down
    ballY -= ballAngle;
    if (ballY <= 10) ballVdir = 0;
  }

  if (ballDir == 0) {  // if moving to the right
    if (ballX >= 120) {
      ballDir = 4;
    } else {
      ballX += ballSpeed;
    }

  } else if (ballDir == 4) {  // if moving to the left
    if (ballX <= 15 && PaddleCollision()) {
      ballDir = 0;//play on
    } else {
      ballX -= ballSpeed;
      if (ballX < 16) {
        //game over??
      }
    }
  }
}

bool PaddleCollision() {
  bool collision = false;
  noInterrupts();
  int paddleY = counter;
  interrupts();
  if (ballY >= paddleY && ballY <= paddleY + paddleWidth) {
    collision = true;
  }
  return collision;
}

void updateEncoder() {
  //read the state of clk

  CLKstate = digitalRead(CLK2);

  if (CLKstate != lastCLKstate) {
    lastCLKstate = CLKstate;
    byte data = digitalRead(DT2);
    if (!data && CLKstate == LOW) {
      counter = counter + 2; //clockwise
    } else if (data && CLKstate == LOW)  {
      counter = counter - 2; //counterclockwise
    }
  }
}

void loop(void) {

  moveBall();
  // picture loop
  u8g.firstPage();
  do {
    draw();
  } while (u8g.nextPage());

}

