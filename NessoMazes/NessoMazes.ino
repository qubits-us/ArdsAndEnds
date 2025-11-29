/*
  Created 2025.25.11 ~q

  NessoMazes - navigate mazes using IMU..
  The nesso should be flat upon startup as this will
  be used as zero..

  maze creating algos research..
  https://weblog.jamisbuck.org/2010/12/27/maze-generation-recursive-backtracking
  fund some c code in above..
  https://gist.github.com/jamis/754545
  the above gives some ruby code..

  The Zeros allow for it to start and return to a stopped state.

  Finishing the maze drops you right back into a new one..
  improvements 2025.27.11 ~q
   added splash screen.
   added system state machine.
   removed all delays.
   added async back ground music.
   added info screen, can turn music on/off and change song..
   also has battery voltage and charge, currently not reporting properly
   but should be fixed in the future??


  No AI was used or consulted during the creation of this software.. :P
  No warranties, no gaurantees..
  Coded with love..
  be it harm none, do as ye wish..
  have fun, code on.. ~q
  www.github.com/qubits-us/

*/

#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>
#include <Arduino_BMI270_BMM150.h>
#include <stack>
#include "Muzak.h"


NessoBattery *battery;

#define TONE_PIN 11

#define DIR_STOP 0
#define DIR_UP 1
#define DIR_DWN 2
#define DIR_LEFT 3
#define DIR_RIGHT 4
//max maze size
#define MAX_HEIGHT 15
#define MAX_WIDTH 20
//size of the maze..
int8_t MazeWidth = 15;
int8_t MazeHeight = 8;
//cell size in pixels..
int8_t CellSize = 15;
//player size in pixels..
int8_t PlayerSize = 2;
//data that needs stacked..
struct MazeCell {
  int8_t x;
  int8_t y;
  int8_t Dirs[4];
  int8_t DirStep;
};
//our cell..
MazeCell Cell;
//player position within the maze..
struct PosPlayer {
  int8_t x;
  int8_t y;
};
PosPlayer PlayerPos;
//dir bit fields or'd into maze cells..
enum { N = 1,
       E = 4,
       S = 2,
       W = 8 };
//directions
int8_t directions[4] = { N, E, S, W };
//direction movements..
int8_t DX[9];
int8_t DY[9];
//opposite directions
int8_t OPPS[9];
//maze Grid..
int8_t MAZE[MAX_WIDTH][MAX_HEIGHT];
//our stack of cells..
std::stack<MazeCell> q;
//need a gap to see outer walls..
int gap = CellSize / 2;

//IMU values
float xValue = 0;
float yValue = 0;
//zero might not be zero
float xZero, yZero;
//tilt threshold
const float thresh = 0.10;

//initial starting pos, top right, exit is bottom left..
int playerX = CellSize / 2 + PlayerSize / 2;
int playerY = CellSize / 2 + PlayerSize / 2;

unsigned long now, lastMove;
bool debug = false;
//maze speed, increase to 1000 when debugging..
unsigned long delayMove = 200;
enum statesSystem { state_Splash = 0,
                    state_Maze = 1,
                    state_Info = 2,
                    state_Wait = 3,
                    state_WaitBtn = 4,
                    state_Launch = 5
} stateSystem = state_Splash;

statesSystem nextState = state_Maze;

struct ScreenButton {
  int x;
  int y;
  int w;
  int h;
};

ScreenButton Buttons[2];

unsigned long waitStart;
unsigned long waitFor;

unsigned long lastButton;
unsigned long btnDebounce = 50;
int lastBtnState = HIGH;

unsigned long lastTouch;
unsigned long touchDebounce = 100;




//music player globals
int Music_Tempo = 240;
int Music_WholeNote = (60000 * 4) / Music_Tempo;
int Music_Note;
int Note_Divider = 0;
int CurrNote = -2;
bool NotePlaying = false;
unsigned long NoteStart, NoteDuration;
bool Music_Enabled = false;
int Music_Song = 0;

const int16_t MusicTempos[] = { 150,
                                300,
                                600,
                                400,
                                400,
                                300,
                                300 };



//Creates the maze..
void CreateMaze() {
  //opposites
  OPPS[N] = S;
  OPPS[S] = N;
  OPPS[E] = W;
  OPPS[W] = E;
  //direction steps..
  DX[N] = 0;
  DX[E] = 1;
  DX[S] = 0;
  DX[W] = -1;
  DY[N] = -1;
  DY[E] = 0;
  DY[S] = 1;
  DY[W] = 0;
  //initialize random..
  randomSeed(analogRead(GROVE_IO_0));
  //out with the old..
  memset(&MAZE[0], 0, sizeof(MAZE));
  //shuffle the directions..
  ShuffleDirections(directions, 4);
  //init our first cell..
  Cell.x = 0;
  Cell.y = 0;
  Cell.DirStep = 0;
  Cell.Dirs[0] = directions[0];
  Cell.Dirs[1] = directions[1];
  Cell.Dirs[2] = directions[2];
  Cell.Dirs[3] = directions[3];
  //push the cell to the stack
  q.push(Cell);
  //carve the maze passages..
  CarveMaze();
  //place our player into a maze cell..
  PlayerPos.x = 0;
  PlayerPos.y = 0;
}

//Shuffles a byte array..
void ShuffleDirections(int8_t *dirs, int size) {
  for (int i = 0; i < size; i++) {
    int r = i + (random() % (size - i));
    int8_t temp = dirs[i];
    dirs[i] = dirs[r];
    dirs[r] = temp;
  }
}

//Carves passages into the maze grid..
//a recursive backtracking algorythm..
//uses iteration in place of recursion..
void CarveMaze() {
  int dx, dy, nx, ny, cx, cy;
  int step, i;
  //loop until stack is empty..
  while (not q.empty()) {
    //get top cell..
    Cell = q.top();
    cx = Cell.x;
    cy = Cell.y;
    step = Cell.DirStep;
    for (i = 0; i < 4; i++)
      directions[i] = Cell.Dirs[i];
    //check all 4 directions..
    for (i = step; i < 4; i++) {
      dx = DX[directions[i]];
      dy = DY[directions[i]];
      nx = cx + dx;
      ny = cy + dy;
      //check valid grid cell..
      if (((nx < MazeWidth) & (nx >= 0)) & ((ny < MazeHeight) & (ny >= 0))) {
        //check if it empty..
        if (MAZE[nx][ny] == 0) {
          //valid cell, let's tunnel into it..
          MAZE[cx][cy] = (int8_t)((int8_t)MAZE[cx][cy] | (int8_t)directions[i]);
          MAZE[nx][ny] = (int8_t)((int8_t)MAZE[nx][ny] | (int8_t)OPPS[directions[i]]);
          //replace old cell..
          q.pop();
          //if we really need to come back here..
          if (i < 3) {
            Cell.DirStep = i;
            q.push(Cell);
          }
          //stack the new cell..
          Cell.x = nx;
          Cell.y = ny;
          Cell.DirStep = 0;
          //shuffle directions..
          ShuffleDirections(directions, 4);
          for (int j = 0; j < 4; j++)
            Cell.Dirs[j] = directions[j];
          q.push(Cell);
          //break out and tunnel into new cell..
          break;
        }  //if empty cell
      }    //valid grid pos
    }      // for all directions..
    //pop the stack if we visited all dirs..
    if (i == 4)
      q.pop();
    //don't be a hog!!
    yield();
  }  //while stack not empty..
}

//prints the maze in ascii to the serial monitor..
//don't look too hard, you may go cross eyed.. :)
void SerialPrintMaze() {
  for (int x = 0; x < (MazeWidth * 2); x++)
    Serial.print("_");
  Serial.println();
  for (int y = 0; y < MazeHeight; y++) {
    Serial.print("|");
    for (int x = 0; x < MazeWidth; x++) {
      Serial.print(((MAZE[x][y] & S) != 0) ? " " : "_");
      if ((MAZE[x][y] & E) != 0) {
        Serial.print((((MAZE[x][y] | MAZE[x + 1][y]) & S) != 0) ? " " : "_");
      } else {
        Serial.print("|");
      }
    }
    Serial.println();
  }
}

//draw the maze to the display..
//draws cell by cell..
void DrawMaze(uint16_t color) {
  int x, y, posx, posy, ex, ey, cell;
  x = 0;
  y = 0;
  cell = 0;
  gap = CellSize / 2;
  posx = 1;
  posy = 1;
  ey = 1;
  ex = CellSize;
  M5.Display.fillScreen(TFT_BLACK);
  for (y = 0; y < MazeHeight; y++) {
    for (x = 0; x < MazeWidth; x++) {
      posx = (x * CellSize) + gap;
      posy = (y * CellSize) + gap;
      ey = posy;
      ex = posx + CellSize;
      //top
      if ((MAZE[x][y] & N) == 0)
        M5.Display.drawLine(posx, posy, ex, ey, color);
      ey = posy + CellSize;
      ex = posx;
      //left
      if ((MAZE[x][y] & W) == 0)
        M5.Display.drawLine(posx, posy, ex, ey, color);
      posy = posy + CellSize;
      ex = posx + CellSize;
      ey = posy;
      //bottom
      if ((MAZE[x][y] & S) == 0)
        M5.Display.drawLine(posx, posy, ex, ey, color);
      posx += CellSize;
      ex = posx;
      ey = posy - CellSize;
      //if not the exit..
      if ((x == MazeWidth - 1) && (y == MazeHeight - 1)) {
        if (debug) Serial.println("Door");
      } else {
        //right
        if ((MAZE[x][y] & E) == 0)
          M5.Display.drawLine(posx, posy, ex, ey, color);
      }
    }
  }
}

void LaunchMaze() {
  M5.Display.fillScreen(TFT_BLACK);
  //speed check vars..
  unsigned long start, end;
  //how much ram free before..
  Serial.printf("Starting heap Available: %d\n", ESP.getFreeHeap());
  start = millis();
  CreateMaze();
  end = millis();
  //how long..
  Serial.printf("Maze Generation Completed Millis: %d\n", (end - start));
  //subtract from above, looks like about 356b, stays the same after
  //multiple maze generations..
  Serial.printf("Ending heap Available: %d\n", ESP.getFreeHeap());
  //print it out to the serial monitor..
  SerialPrintMaze();
  //draw it on the screen..
  DrawMaze(YELLOW);

  stateSystem = state_Maze;
}

//step the maze..
void StepMaze() {
  int dir = readIMU();
  drawPlayer(WHITE);

  // Check if the user's next move will intersect with a wall
  bool canMove = dir != DIR_STOP;
  int nextX = PlayerPos.x;
  int nextY = PlayerPos.y;

  //navigate maze..
  switch (dir) {
    case DIR_LEFT:
      nextX -= 1;
      if (nextX < 0) {
        canMove = false;
      } else {
        if ((MAZE[PlayerPos.x][PlayerPos.y] & W) == 0) canMove = false;
      }
      break;
    case DIR_UP:
      nextY -= 1;
      if (nextY < 0) {
        canMove = false;
      } else {
        if ((MAZE[PlayerPos.x][PlayerPos.y] & N) == 0) canMove = false;
      }
      break;
    case DIR_RIGHT:
      nextX += 1;
      if (nextX > MazeWidth - 1) {
        canMove = false;
      } else {
        if ((MAZE[PlayerPos.x][PlayerPos.y] & E) == 0) canMove = false;
      }
      break;
    case DIR_DWN:
      nextY += 1;
      if (nextY > MazeHeight - 1) {
        canMove = false;
      } else {
        if ((MAZE[PlayerPos.x][PlayerPos.y] & S) == 0) canMove = false;
      }
      break;
  }

  // move the user if allowed
  if (canMove) {
    //erase last pos..
    drawPlayer(BLACK);
    PlayerPos.x = nextX;
    PlayerPos.y = nextY;
    drawPlayer(WHITE);
  }
}



//draw player into a new cell of the maze..
//also checks for completion and creates new maze..
void drawPlayer(uint16_t color) {
  //are we done..
  if (PlayerPos.x == MazeWidth - 1 && PlayerPos.y == MazeHeight - 1) {
    LaunchMaze();
  } else {
    //translate grid to screen cords..
    playerX = (PlayerPos.x * CellSize) + (CellSize / 2) + (PlayerSize / 2) + gap;
    playerY = (PlayerPos.y * CellSize) + (CellSize / 2) + (PlayerSize / 2) + gap;
    //draw the player..
    M5.Display.fillCircle(playerX, playerY, PlayerSize, color);
  }
}


//Read the IMU accelerator and return a direction..
int readIMU() {
  //default to stopped
  int result = DIR_STOP;


  if (IMU.accelerationAvailable()) {
    float zValue;
    IMU.readAcceleration(xValue, yValue, zValue);
    if (debug) {
      Serial.print("X:");
      Serial.print(xValue);
      Serial.print(" Y:");
      Serial.print(yValue);
    }

    if (xValue < (xZero - thresh)) {
      //left
      result = DIR_LEFT;
      if (debug) Serial.print(" Left");

    } else if (xValue > (xZero + thresh)) {
      //right
      result = DIR_RIGHT;
      if (debug) Serial.print(" Right");
    }

    if (result == DIR_STOP) {
      if (yValue < (yZero - thresh)) {
        //down
        result = DIR_DWN;
        if (debug) Serial.print(" Down");
      } else if (yValue > (yZero + thresh)) {
        //up
        result = DIR_UP;
        if (debug) Serial.print(" Up");
      }
    }
  }

  if (debug) {
    if (result == DIR_STOP) Serial.print(" Stop");
    Serial.println();
  }
  return result;
}




void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  if (!IMU.begin()) {
    Serial.println("IMU failed!!");
    while (1)
      ;
  }

  delay(2000);  //nap while imu warms up a bit..
  float zValue;
  IMU.readAcceleration(xZero, yZero, zValue);

  Serial.println("**** Nesso Mazes ~q ****");
  //hope nesso is laying flat.. :)
  Serial.print("X zero:");
  Serial.println(xZero);
  Serial.print("Y zero:");
  Serial.println(yZero);

  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.init();
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);

  //tried a few easy things..
  battery = new NessoBattery();
  //tried unified as well..
  Serial.printf("Batt Level: %d\n", M5.Power.getBatteryLevel());
  Serial.print("Batt Volts: ");
  Serial.println(M5.Power.getBatteryVoltage());
}

void loop() {

  now = millis();
  M5.update();

  lgfx::touch_point_t tp;
  if (M5.Display.getTouch(&tp)) {
    if (now - lastTouch >= touchDebounce) {
      //got a touch..
      lastTouch = now;
      if (stateSystem == state_WaitBtn) {
        //info screen is up, process touches..
        CheckScreenTouch(tp.x, tp.y);
      }
    }
  }

  int b = M5.BtnA.wasClicked();
  if (now - lastButton >= btnDebounce) {
    if (b) {
      //start debounce
      lastButton = now;
      //press and release buttton
      switch (stateSystem) {
        case state_Maze:
          stateSystem = state_Info;
          break;
        case state_WaitBtn:
          DrawMaze(YELLOW);
          stateSystem = state_Maze;
          break;
      }
    }
  }

  if (Music_Enabled) {  //play a tune..
    Music_Tempo = MusicTempos[Music_Song];
    Music_WholeNote = (60000 * 4) / Music_Tempo;
    switch (Music_Song) {
      case 0:
        PlayMusic(furelise, sizeof(furelise) / sizeof(furelise[0]));
        break;
      case 1:
        PlayMusic(minuet, sizeof(minuet) / sizeof(minuet[0]));
        break;
      case 2:
        PlayMusic(cannon, sizeof(cannon) / sizeof(cannon[0]));
        break;
      case 3:
        PlayMusic(odetojoy, sizeof(odetojoy) / sizeof(odetojoy[0]));
        break;
      case 4:
        PlayMusic(brahmslullaby, sizeof(brahmslullaby) / sizeof(brahmslullaby[0]));
        break;
      case 5:
        PlayMusic(starwars, sizeof(starwars) / sizeof(starwars[0]));
        break;
      case 6:
        PlayMusic(imperialmarch, sizeof(imperialmarch) / sizeof(imperialmarch[0]));
        break;
    }
  }

  switch (stateSystem) {
    case state_Maze:
      if (now - lastMove >= delayMove) {
        lastMove = now;
        StepMaze();
      }
      break;
    case state_Splash:
      ScreenSplash();
      break;
    case state_Info:
      ScreenInfo();
      break;
    case state_Wait:
      if (now - waitStart >= waitFor) {
        stateSystem = nextState;
      }
      break;
    case state_Launch:
      LaunchMaze();
      break;
  }
  yield();
}


void ScreenSplash() {
  int tw, th, tx, ty, sh, sw;
  sh = M5.Display.height();
  sw = M5.Display.width();
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(2);
  th = M5.Display.fontHeight();
  tw = M5.Display.textWidth("Nesso Mazes");
  tx = (M5.Display.width() / 2) - (tw / 2);
  ty = (M5.Display.height() / 2) - (th / 2);
  CreateMaze();
  DrawMaze(YELLOW);
  M5.Display.fillRect(tx - 6, ty - 6, tw + 12, th + 12, BLACK);
  M5.Display.drawRect(tx - 6, ty - 6, tw + 12, th + 12, YELLOW);
  M5.Display.drawString("Nesso Mazes", tx, ty);
  waitStart = now;
  waitFor = 5000;
  stateSystem = state_Wait;
  nextState = state_Launch;
}



void ScreenInfo() {
  int tw, th, tx, ty, gap, sh, sw, nw;

  sh = M5.Display.height();
  sw = M5.Display.width();

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE, BLACK);
  //x,y
  tx = 10;
  ty = 10;
  gap = 2;
  tw = M5.Display.textWidth("Music");
  th = M5.Display.fontHeight();
  M5.Display.drawRect(tx, ty, tw + 12, th + 12, YELLOW);
  M5.Display.drawString("Music", tx + 6, ty + 6);
  ty += th + 12 + gap;
  M5.Display.drawRect(tx, ty, tw + 12, th + 24, YELLOW);
  Buttons[0].x = tx;
  Buttons[0].y = ty;
  Buttons[0].w = tw + 12;
  Buttons[0].h = th + 24;
  if (Music_Enabled) {
    nw = M5.Display.textWidth("On");
    M5.Display.drawString("On", (tx + 6) + (nw / 2), ty + 12);
  } else {
    nw = M5.Display.textWidth("Off");
    M5.Display.drawString("Off", (tx + 6) + (nw / 2), ty + 12);
  }
  tx += tw + 12 + gap;
  ty = 10;
  M5.Display.drawRect(tx, ty, tw + 12, th + 12, YELLOW);
  M5.Display.drawString("Song", tx + 12, ty + 6);
  ty += th + 12 + gap;
  M5.Display.drawRect(tx, ty, tw + 12, th + 24, YELLOW);
  Buttons[1].x = tx;
  Buttons[1].y = ty;
  Buttons[1].w = tw + 12;
  Buttons[1].h = th + 24;
  nw = M5.Display.textWidth("0");

  M5.Display.drawNumber(Music_Song + 1, tx + (nw / 2) + (tw / 2), ty + 12);
  tx += tw + 12 + gap;
  ty = 10;
  M5.Display.drawRect(tx, ty, tw + 12, th + 12, YELLOW);
  M5.Display.drawString("BattV", tx + 6, ty + 6);
  ty += th + 12 + gap;
  M5.Display.drawRect(tx, ty, tw + 12, th + 24, YELLOW);

  nw = M5.Display.textWidth("9.9");
  M5.Display.drawFloat(battery->getVoltage(), 1, (tx) + (nw / 2), ty + 12);

  tx = 10;
  ty += th + 24 + gap;
  M5.Display.drawRect(tx, ty, sw - (tx * 2), (sh - ty) - 10, YELLOW);
  nw = (sw - (tx * 2)) - 4;
  int cl = battery->getChargeLevel();
  if (cl < 100) {
    float multi = cl / 100.0;
    nw = nw * multi;
  }
  M5.Display.fillRect(tx + 2, ty + 2, nw, (sh - ty) - 14, GREEN);
  M5.Display.drawNumber(cl, (sw / 2) - (tw / 2) + 5, ty + 12);
  stateSystem = state_WaitBtn;
}

bool TouchArea(int px, int py, int x, int y, int w, int h) {
  bool result = false;
  int x2 = x + w;
  int y2 = y + h;
  int touch_x = px;
  int touch_y = py;
  if ((touch_x >= x) && (touch_x <= x2) && (touch_y >= y) && (touch_y <= y2)) result = true;
  if (debug) {
    Serial.printf("X: %d , X1: %d, Y: %d, Y1: %d\n", touch_x, x, touch_y, y);
    Serial.printf("Matched :%d", result);
  }
  return result;
}

void CheckScreenTouch(int tx, int ty) {
  int nw;
  //only have 1 screen to watch..

  if (TouchArea(tx, ty, Buttons[0].x, Buttons[0].y, Buttons[0].w, Buttons[0].h)) {
    //touch on button 1 - music on/off toggle
    if (debug)
      Serial.println("Button 1 touched..");

    if (Music_Enabled) {
      Music_Enabled = false;
      noTone(TONE_PIN);
      nw = M5.Display.textWidth("Off");
      M5.Display.setTextColor(BLACK, BLACK);
      M5.Display.drawString(" On", Buttons[0].x + 6 + (nw / 2), Buttons[0].y + 12);
      M5.Display.setTextColor(WHITE, BLACK);
      M5.Display.drawString("Off", Buttons[0].x + 6 + (nw / 2), Buttons[0].y + 12);
    } else {
      Music_Enabled = true;
      nw = M5.Display.textWidth("On");
      M5.Display.setTextColor(BLACK, BLACK);
      M5.Display.drawString("Off", Buttons[0].x + 6 + (nw / 2), Buttons[0].y + 12);
      M5.Display.setTextColor(WHITE, BLACK);
      M5.Display.drawString(" On", Buttons[0].x + 6 + (nw / 2), Buttons[0].y + 12);
    }

  } else if (TouchArea(tx, ty, Buttons[1].x, Buttons[1].y, Buttons[1].w, Buttons[1].h)) {
    //touch on button 2 - song button select
    if (debug)
      Serial.println("Button 2 touched..");
    bool isEnabled = Music_Enabled;
    Music_Enabled = false;
    Music_Song++;
    if (Music_Song > (sizeof(MusicTempos) / sizeof(MusicTempos[0])) - 1) Music_Song = 0;
    CurrNote = -2;
    Music_Enabled = isEnabled;
    M5.Display.waitDisplay();
    //remove old text..
    M5.Display.fillRect(Buttons[1].x + 2, Buttons[1].y + 2, Buttons[1].w - 4, Buttons[1].h - 4, BLACK);
    nw = M5.Display.textWidth("0");
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.drawNumber(Music_Song + 1, Buttons[1].x + (nw / 2) + ((Buttons[1].w - 12) / 2), Buttons[1].y + 12);
    M5.Display.display();
  }
}

//async tone music player..
void PlayMusic(const int16_t song[], int elements) {
  static bool resting = false;
  if (now - NoteStart >= NoteDuration) {
    if (NotePlaying) {
      if (!resting) {
        noTone(TONE_PIN);
      } else resting = false;
      NotePlaying = false;
      NoteStart = now;
    } else {
      CurrNote += 2;
      if (CurrNote < (elements - 1)) {
        Music_Note = song[CurrNote];
        Note_Divider = song[CurrNote + 1];
        if (Note_Divider > 0) {
          NoteDuration = Music_WholeNote / Note_Divider;
        } else {
          NoteDuration = Music_WholeNote / abs(Note_Divider);
          NoteDuration *= 1.5;
        }
        if (Music_Note != REST) {
          tone(TONE_PIN, Music_Note);
        } else resting = true;
        NoteStart = now;
        NotePlaying = true;
      } else {
        //pause for a sec, then loop..
        noTone(TONE_PIN);
        NoteStart = now;
        NoteDuration = 1000;
        //Music_Song++;
        //if (Music_Song > (sizeof(MusicTempos) / sizeof(MusicTempos[0])) - 1) Music_Song = 0;
        CurrNote = -2;
      }
    }
  }
}
