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

  No AI was used or consulted during the creation of this software.. :P
  No warranties, no gaurantees..
  Coded with love..
  be it harm none, do as ye wish..
  have fun, code on.. ~q
  https://github.com/qubits-us/ArdsAndEnds

*/

#include <M5GFX.h>
#include <Arduino_BMI270_BMM150.h>
#include <stack>

M5GFX display;

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

int gap = CellSize / 2;

//IMU values
float xValue = 0;
float yValue = 0;
//zero might not be zero
float xZero, yZero;
//tilt threshold
const float thresh = 0.10;

//initial starting pos
int playerX = CellSize / 2 + PlayerSize / 2;
int playerY = CellSize / 2 + PlayerSize / 2;

bool mazeRed = false;
bool debug = false;

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
  randomSeed(analogRead(0));
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
  for (y = 0; y < MazeHeight; y++) {
    for (x = 0; x < MazeWidth; x++) {
      posx = (x * CellSize) + gap;
      posy = (y * CellSize) + gap;
      ey = posy;
      ex = posx + CellSize;
      //top
      if ((MAZE[x][y] & N) == 0)
        display.drawLine(posx, posy, ex, ey, color);
      ey = posy + CellSize;
      ex = posx;
      //left
      if ((MAZE[x][y] & W) == 0)
        display.drawLine(posx, posy, ex, ey, color);
      posy = posy + CellSize;
      ex = posx + CellSize;
      ey = posy;
      //bottom
      if ((MAZE[x][y] & S) == 0)
        display.drawLine(posx, posy, ex, ey, color);
      posx += CellSize;
      ex = posx;
      ey = posy - CellSize;
      //if not the exit..
      if ((x == MazeWidth - 1) && (y == MazeHeight - 1)) {
        if (debug) Serial.println("Door");
      } else {
        //right
        if ((MAZE[x][y] & E) == 0)
          display.drawLine(posx, posy, ex, ey, color);
      }
    }
  }
}

//draw player into a new cell of the maze..
//also checks for completion and creates new maze..
void drawPlayer(uint16_t color) {
  //are we done..
  if (PlayerPos.x == MazeWidth - 1 && PlayerPos.y == MazeHeight - 1) {
    //maze completed..
    display.fillScreen(TFT_BLACK);
    //speed check vars..
    unsigned long start, end;
    //how much ram free before..
    Serial.printf("Starting heap Available: %d\n", ESP.getFreeHeap());
    start = millis();
    CreateMaze();
    end = millis();
    //how long..
    Serial.printf("Maze Generation Completed Millis: %d\n", (end - start));
    //subtract from above, loooks like about 356b, stays the same after
    //multiple maze generations..
    Serial.printf("Ending heap Available: %d\n", ESP.getFreeHeap());
    //print it out to the serial monitor..
    SerialPrintMaze();
    //draw it on the screen..
    DrawMaze(YELLOW);
  } else {
    //translate grid to screen cords..
    playerX = (PlayerPos.x * CellSize) + (CellSize / 2) + (PlayerSize / 2) + gap;
    playerY = (PlayerPos.y * CellSize) + (CellSize / 2) + (PlayerSize / 2) + gap;
    //draw the player..
    display.fillCircle(playerX, playerY, PlayerSize, color);
  }
}


//Read the IMU accelerator and return a direction..
int readIMU() {
  //default to stopped
  int result = DIR_STOP;
  if (IMU.accelerationAvailable()) {
    // read the IMU accel
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
  delay(2000);  //nap
  if (!IMU.begin()) {
    Serial.println("Failed to init the IMU!!");
    while (1)
      ;  //stop
  }
  delay(2000);   //nap while imu warms up a bit..
  float zValue;  //throw away..
  IMU.readAcceleration(xZero, yZero, zValue);

  Serial.println("**** Nesso Mazes ~q ****");
  //hope nesso is laying flat.. :)
  Serial.print("X zero:");
  Serial.println(xZero);
  Serial.print("Y zero:");
  Serial.println(yZero);

  display.begin();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);
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
}

void loop() {


  int dir = readIMU();
  drawPlayer(WHITE);
  //maze speed, slower for debug
  if (debug) delay(1000);
  else delay(200);

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
  }
}
