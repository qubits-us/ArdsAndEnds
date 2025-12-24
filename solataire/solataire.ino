#include <M5Unified.h>
#include <M5GFX.h>
#include<time.h>
#include "cards.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

M5GFX Display;


int8_t ButtonWidth = 40;
int8_t ButtonHeight = 30;
int8_t CardWidth = 60;
int8_t CardHeight = 84;
int8_t CardBack = 52;
int8_t CardOL = 20;
int8_t Border = 20;
int8_t Gap = 30;

int TableWidth = (CardWidth * 7) + (Gap * 6) + (Border * 2);
int TableHeight = (CardHeight * 2) + (Border * 2) + (CardOL * 12) + (Gap *2) + ButtonHeight;
int TableLeft = (SCREEN_WIDTH - TableWidth) / 2;
int TableTop = (SCREEN_HEIGHT - TableHeight) / 2;

int8_t Deck[52];

int8_t HiddenTabs[6][6];
int8_t Tableau[7][13];
int8_t Foundations[4][13];
int8_t Stock[24];
int8_t Waste[24];

int8_t StockCount = 0;
int8_t WasteCount = 0;
int8_t CurrentCard = 0;
int8_t TableauCounts[7] = {0, 0, 0, 0, 0, 0, 0};
int8_t HiddenCounts[7] = {0, 0, 0, 0, 0, 0, 0};
int8_t FoundationCounts[4] = {0, 0, 0, 0};
int8_t SelectedCard = -1;
int8_t SelectedArea = -1;
int8_t SelectCount = 0;

struct TouchSpot
{
  int x;
  int y;
  int w;
  int h;
  int area;
};

TouchSpot Spots[13];
TouchSpot Button;

struct SelectionRect
{
  int x;
  int y;
  int w;
  int h;
};

SelectionRect SelectRect;

unsigned long lastTouch;
unsigned long touchDebounce = 200;
unsigned long now;

bool debug = true;

// forward declarations..
void CutDeck(int8_t *deck, int size);
void shuffle(int8_t *array, int size);
void SetupTable();
void StockFlip();
bool TouchArea(int px, int py, int x, int y, int w, int h);
void CheckScreenTouch(int tx, int ty);
void HandleTouch(int spot, int y);
bool CheckFoundationMove(int f);
void MoveToFoundations(int f);
void SelectFoundation(int f);
int GetCardColor(int acard);
int GetCardNumber(int acard);
bool CheckTableauMove(int a);
void MoveToTableau(int a);
void SelectTableau(int a, int y);
void DrawSelection();


void SetupTable()
{
  // clear out the old values..
  memset(HiddenTabs, -1, sizeof(HiddenTabs));
  memset(Tableau, -1, sizeof(Tableau));
  memset(Foundations, -1, sizeof(Foundations));
  memset(Stock, -1, sizeof(Stock));
  memset(Waste, -1, sizeof(Waste));
  memset(HiddenCounts, 0, sizeof(HiddenCounts));
  StockCount = 0;
  WasteCount = 0;
  CurrentCard = 0;
  for (int i = 0; i < sizeof(TableauCounts); i++)
  {
    TableauCounts[i] = 1;
    if (i < sizeof(FoundationCounts))
      FoundationCounts[i] = 0;
  }

  shuffle(Deck, sizeof(Deck));
  CutDeck(Deck,sizeof(Deck));
  shuffle(Deck, sizeof(Deck));
  CutDeck(Deck,sizeof(Deck));

  // fill the tableau
  int c = 0;
  for (int j = 0; j < 7; j++)
  {
    Tableau[j][0] = Deck[c];
    c++;
    for (int i = j; i < 6; i++)
    {
      HiddenTabs[i][j] = Deck[c];
      HiddenCounts[i + 1]++;
      c++;
    }
  }
  // fill stock
  StockCount = 52 - c;
  int p = 0;
  for (int i = c; i < 52; i++)
  {
    Stock[p] = Deck[c];
    p++;
    c++;
  }

  M5.Display.fillScreen(TFT_DARKGREEN);

  int sw, sh;

  sw = Display.width();
  sh = Display.height();

  int x = Border + TableLeft;
  int y = Border + TableTop;
  Button.x = x;
  Button.y = y;
  Button.w = ButtonWidth;
  Button.h = ButtonHeight;
  M5.Display.fillRoundRect(x,y, ButtonWidth, ButtonHeight, 8 , TFT_LIGHTGRAY);
  M5.Display.setTextColor(TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.drawString("New",x+3,y+8);
  M5.Display.drawRoundRect(x, y, ButtonWidth, ButtonHeight,8, TFT_YELLOW);
  y+=ButtonHeight+Gap;

  Spots[0].y = y;
  Spots[0].x = x;
  Spots[0].h = CardHeight;
  Spots[0].w = CardWidth;
  Spots[0].area = 1;
  M5.Display.pushImage(x, y, 60, 84, card[CardBack]);
  M5.Display.drawRect(x, y, CardWidth, CardHeight, TFT_YELLOW);
  x += CardWidth + Gap;
  Spots[1].y = y;
  Spots[1].x = x;
  Spots[1].h = CardHeight;
  Spots[1].w = CardWidth;
  Spots[1].area = 2;
  M5.Display.drawRect(x, y, CardWidth, CardHeight, TFT_YELLOW);
  x += (CardWidth + Gap) * 2;

  for (int i = 0; i < 4; i++)
  {
    Spots[i + 2].y = y;
    Spots[i + 2].x = x;
    Spots[i + 2].h = CardHeight;
    Spots[i + 2].w = CardWidth;
    Spots[i + 2].area = i + 3;
    M5.Display.drawRect(x, y, CardWidth, CardHeight, TFT_YELLOW);
    x += CardWidth + Gap;
  }
  x = Border + TableLeft;
  y = CardHeight + Border + Gap + TableTop+ButtonHeight+Gap;

  for (int i = 0; i < 7; i++)
  {
    Spots[i + 6].y = y;
    Spots[i + 6].x = x;
    Spots[i + 6].h = CardHeight;
    Spots[i + 6].w = CardWidth;
    Spots[i + 6].area = i + 7;
    if (i > 0)
    {
      M5.Display.pushImage(x, y, CardWidth, CardHeight, card[CardBack]);
    }
    M5.Display.drawRect(x, y, CardWidth, CardHeight, TFT_YELLOW);
    M5.Display.pushImage(x, y + CardOL, CardWidth, CardHeight, card[Tableau[i][0]]);
    M5.Display.drawRect(x, y + CardOL, CardWidth, CardHeight, TFT_YELLOW);
    x += CardWidth + Gap;
  }
  
}

void StockFlip()
{
  if (SelectedCard > -1){
  SelectedCard = -1;
  SelectedArea = -1;
  SelectCount = 0;
  DrawSelection();
  }
  if (StockCount == 0)
  {
    // restock..
    int flip = WasteCount - 1;
    for (int i = 0; i < WasteCount; i++)
    {
      Stock[i] = Waste[flip--];
    }
    StockCount = WasteCount;
    WasteCount = 0;
    M5.Display.pushImage(Spots[0].x, Spots[0].y, CardWidth, CardHeight, card[CardBack]);
    M5.Display.drawRoundRect(Spots[0].x, Spots[0].y, CardWidth, CardHeight, 4, TFT_YELLOW);
  }
  // flip the next stock card over..
  Waste[WasteCount] = Stock[StockCount - 1];
  WasteCount++;
  Stock[StockCount - 1] = -1;
  if (StockCount > 0)
    StockCount--;
  if (StockCount == 0)
  {
    // no more stock..
    M5.Display.fillRect(Spots[0].x, Spots[0].y, CardWidth, CardHeight, TFT_DARKGREEN);
    M5.Display.drawRoundRect(Spots[0].x, Spots[0].y, CardWidth, CardHeight, 4, TFT_YELLOW);
  }
  M5.Display.pushImage(Spots[1].x, Spots[1].y, 60, 84, card[Waste[WasteCount - 1]]);
  M5.Display.drawRoundRect(Spots[1].x, Spots[1].y, 60, 84, 4, TFT_YELLOW);
}

void SelectWaste()
{
  if (WasteCount > 0)
  {
    SelectedCard = Waste[WasteCount - 1];
    SelectedArea = 2;
    SelectCount = 1;
    SelectRect.x = Spots[1].x;
    SelectRect.y = Spots[1].y;
    SelectRect.w = Spots[1].w;
    SelectRect.h = Spots[1].h;
    DrawSelection();
  }
}

bool CheckFoundationMove(int f)
{
  bool result = false;
  int nextCard;
  if (FoundationCounts[f - 3] > 0)
  {
    if (FoundationCounts[f - 3] == 1)
    {
      // it's the ace..
      nextCard = Foundations[f - 3][FoundationCounts[f - 3] - 1] - 12;
    }
    else
    {
      nextCard = Foundations[f - 3][FoundationCounts[f - 3] - 1] + 1;
    }
    if (SelectedCard == nextCard)
      result = true;
  }
  else
  {
    // looking for an ace
    switch (SelectedCard)
    {
    case 12:
    case 25:
    case 38:
    case 51:
      result = true;
      break;
    }
  }
  return result;
}

void MoveToFoundations(int f)
{
  if (SelectedArea == 2 && SelectedCard > -1)
  {
    // move a waste card..
    Foundations[f - 3][FoundationCounts[f - 3]] = Waste[WasteCount - 1];
    FoundationCounts[f - 3]++;
    Waste[WasteCount - 1] = -1;
    WasteCount--;
    if (WasteCount < 1)
    {
      M5.Display.fillRect(Spots[1].x, Spots[1].y, CardWidth, CardHeight, TFT_DARKGREEN);
      M5.Display.drawRoundRect(Spots[1].x, Spots[1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
    }
    else
    {
      M5.Display.pushImage(Spots[1].x, Spots[1].y, CardWidth, CardHeight, card[Waste[WasteCount - 1]]);
      M5.Display.drawRoundRect(Spots[1].x, Spots[1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
    }
    M5.Display.pushImage(Spots[f - 1].x, Spots[f - 1].y, CardWidth, CardHeight, card[Foundations[f - 3][FoundationCounts[f - 3] - 1]]);
    M5.Display.drawRoundRect(Spots[f - 1].x, Spots[f - 1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
    SelectedCard = -1;
    SelectedArea = -1;
    SelectCount = 0;
    DrawSelection();
  }
  else
  {
    if (SelectedArea > 6)
    {
      // moving from the tasbleau
      if (SelectedCard == Tableau[SelectedArea - 7][TableauCounts[SelectedArea - 7] - 1])
      {
        // can only move a top card..
        Foundations[f - 3][FoundationCounts[f - 3]] = Tableau[SelectedArea - 7][TableauCounts[SelectedArea - 7] - 1];
        FoundationCounts[f - 3]++;
        Tableau[SelectedArea - 7][TableauCounts[SelectedArea - 7] - 1] = -1;
        TableauCounts[SelectedArea - 7]--;

        int newy = CardOL * TableauCounts[SelectedArea - 7];
        Spots[SelectedArea - 1].h = CardHeight + newy;
        // erase old card first
        M5.Display.fillRect(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y + newy + CardOL, CardWidth, CardHeight, TFT_DARKGREEN);
        // redraw last card in pile..
        if (TableauCounts[SelectedArea - 7] > 0)
          M5.Display.pushImage(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y + newy, CardWidth, CardHeight, card[Tableau[SelectedArea - 7][TableauCounts[SelectedArea - 7] - 1]]);
        else if (SelectedArea > 7 && HiddenCounts[SelectedArea - 7] > 0) // do we have something??
          M5.Display.pushImage(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y + newy, CardWidth, CardHeight, card[CardBack]);
        M5.Display.drawRoundRect(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y + newy, CardWidth, CardHeight, 4, TFT_YELLOW);
        // draw card in foundation
        M5.Display.pushImage(Spots[f - 1].x, Spots[f - 1].y, CardWidth, CardHeight, card[Foundations[f - 3][FoundationCounts[f - 3] - 1]]);
        M5.Display.drawRoundRect(Spots[f - 1].x, Spots[f - 1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
        SelectedCard = -1;
        SelectedArea = -1;
      }
      else
      {
        // just unselect pile
        SelectedCard = -1;
        SelectedArea = -1;
        SelectCount = 0;
        DrawSelection();
      }
    }
    else
    {
      // just unselect pile
      SelectedCard = -1;
      SelectedArea = -1;
      SelectCount = 0;
      DrawSelection();
    }
  }
}

void SelectFoundation(int f)
{
  // select a foundation card..
  if (FoundationCounts[f - 3] > 0)
  {
    SelectedCard = Foundations[f - 3][FoundationCounts[f - 3] - 1];
    SelectedArea = f;
    SelectCount = 1;
    SelectRect.x = Spots[f - 1].x;
    SelectRect.y = Spots[f - 1].y;
    SelectRect.w = Spots[f - 1].w;
    SelectRect.h = Spots[f - 1].h;
    DrawSelection();
  }
}

int GetCardColor(int acard)
{
  int result = -1;
  if (acard < 13)
    result = 0;
  else if (acard < 39)
    result = 1;
  else
    result = 0;
  return result;
}

int GetCardNumber(int acard)
{
  int result = -1;
  // get ordinal postion within suit..
  if (acard < 13)
    result = acard+2;
  else if (acard < 26)
    result = acard - 11;
  else if (acard < 39)
    result = acard - 24;
  else
    result = acard - 37;

   printf("Orig #: %d new #: %d\n",acard,result);   
 return result;   
}

bool CheckTableauMove(int a)
{
  // check for valid move
  bool result = false;
  // no cards on table, any card should do..
  if (TableauCounts[a - 7] == 0)
  {
    result = true;
    return result; // outa here
  }
  int pileColor = GetCardColor(Tableau[a - 7][TableauCounts[a - 7] - 1]);
  if (GetCardColor(SelectedCard) == pileColor)
  {
    int acolor = GetCardColor(SelectedCard);
    result = false;
    return result;
  }
  int pileCard = GetCardNumber(Tableau[a - 7][TableauCounts[a - 7] - 1]);
  // next card should be one less than the current pile card..
  result = pileCard == (GetCardNumber(SelectedCard) + 1);
  return result;
}

void MoveToTableau(int a)
{
  if (SelectedArea == 2)
  {
    // moving from waste..
    Tableau[a - 7][TableauCounts[a - 7]] = Waste[WasteCount - 1];
    TableauCounts[a - 7]++;
    Waste[WasteCount - 1] = -1;
    WasteCount--;
    if (WasteCount < 1)
    {
      M5.Display.fillRect(Spots[1].x, Spots[1].y, CardWidth, CardHeight, TFT_DARKGREEN);
      M5.Display.drawRoundRect(Spots[1].x, Spots[1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
    }
    else
    {
      M5.Display.pushImage(Spots[1].x, Spots[1].y, CardWidth, CardHeight, card[Waste[WasteCount - 1]]);
      M5.Display.drawRoundRect(Spots[1].x, Spots[1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
    }
    int newy = CardOL * TableauCounts[a - 7];
    Spots[a - 1].h = CardHeight + newy;
    M5.Display.pushImage(Spots[a - 1].x, Spots[a - 1].y + newy, CardWidth, CardHeight, card[Tableau[a - 7][TableauCounts[a - 7] - 1]]);
    M5.Display.drawRoundRect(Spots[a - 1].x, Spots[a - 1].y + newy, CardWidth, CardHeight, 4, TFT_YELLOW);
    SelectedCard = -1;
    SelectedArea = -1;
    SelectCount = 0;
    DrawSelection();
  }
  else if (SelectedArea > 2 && SelectedArea < 7)
  {
    // moving from foundations..
    Tableau[a - 7][TableauCounts[a - 7]] = Foundations[SelectedArea - 3][FoundationCounts[SelectedArea - 3] - 1];
    TableauCounts[a - 7]++;
    Foundations[SelectedArea - 3][FoundationCounts[SelectedArea - 3] - 1] = -1;
    FoundationCounts[SelectedArea - 3]--;
    if (FoundationCounts[SelectedArea - 3] < 1)
    {
      M5.Display.fillRect(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y, CardWidth, CardHeight, TFT_DARKGREEN);
      M5.Display.drawRoundRect(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
    }
    else
    {
      M5.Display.pushImage(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y, CardWidth, CardHeight,card[Foundations[SelectedArea - 3][FoundationCounts[SelectedArea - 3] - 1]]);
      M5.Display.drawRoundRect(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
    }
    int newy = CardOL * TableauCounts[a - 7];
    Spots[a - 1].h = CardHeight + newy;
    M5.Display.pushImage(Spots[a - 1].x, Spots[a - 1].y + newy, CardWidth, CardHeight, card[Tableau[a - 7][TableauCounts[a - 7] - 1]]);
    M5.Display.drawRoundRect(Spots[a - 1].x, Spots[a - 1].y + newy, CardWidth, CardHeight, 4, TFT_YELLOW);
    SelectedCard = -1;
    SelectedArea = -1;
    SelectCount = 0;
    DrawSelection();
    //   M5.Display.pushM5.Display(0, 0);
  }
  else if (SelectedArea > 6 && SelectedArea < 14 && SelectedArea != a)
  {
    // moving from pile to pile..
    int c = SelectCount;
    int newy;
    for (int i = 0; i < SelectCount; i++)
    {
      // add cards
      Tableau[a - 7][TableauCounts[a - 7]] = Tableau[SelectedArea - 7][(TableauCounts[SelectedArea - 7] - c)];
      TableauCounts[a - 7]++;
      newy = CardOL * TableauCounts[a - 7];
      Spots[a - 1].h = CardHeight + newy;
      M5.Display.pushImage(Spots[a - 1].x, Spots[a - 1].y + newy, CardWidth, CardHeight, card[Tableau[a - 7][TableauCounts[a - 7] - 1]]);
      M5.Display.drawRoundRect(Spots[a - 1].x, Spots[a - 1].y + newy, CardWidth, CardHeight, 4, TFT_YELLOW);
      c--;
    }
    c = SelectCount;
   // newy = Spots[a-1].h;
    for (int i = 0; i < SelectCount; i++)
    {
      // remove cards
      Tableau[SelectedArea - 7][(TableauCounts[SelectedArea - 7] - c)] = -1;
      TableauCounts[SelectedArea - 7]--;
      newy = CardOL * TableauCounts[SelectedArea - 7];
     // newy-= CardOL;
     if (TableauCounts[SelectedArea -7]>1)
      Spots[SelectedArea - 1].h = CardHeight + newy; else
      Spots[SelectedArea - 1].h = CardHeight;
      // erase old card first
      M5.Display.fillRect(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y + newy + CardOL, CardWidth, CardHeight, TFT_DARKGREEN);
      // redraw last card in pile..
      if (TableauCounts[SelectedArea - 7] > 0)
        M5.Display.pushImage(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y + newy, CardWidth, CardHeight, card[Tableau[SelectedArea - 7][TableauCounts[SelectedArea - 7] - 1]]);
      else if (SelectedArea > 7 && HiddenCounts[SelectedArea - 7] > 0) // do we have something??
        M5.Display.pushImage(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y + newy, CardWidth, CardHeight, card[CardBack]);
      M5.Display.drawRoundRect(Spots[SelectedArea - 1].x, Spots[SelectedArea - 1].y + newy, CardWidth, CardHeight, 4, TFT_YELLOW);
      c--;
    }
    SelectedCard = -1;
    SelectedArea = -1;
    SelectCount = 0;
  }
  else
  {
    SelectedCard = -1;
    SelectedArea = -1;
    SelectCount = 0;
    DrawSelection();
  }
}

void SelectTableau(int a, int y)
{
  if (TableauCounts[a - 7] > 0)
  {
    if (TableauCounts[a - 7] > 1)
    {
      int newy = Spots[a - 1].y + (TableauCounts[a - 7] * CardOL);
      for (int i = 0; i < TableauCounts[a - 7]; i++)
      {
        if (y >= newy)
        {
          SelectedCard = Tableau[a - 7][(TableauCounts[a - 7] - 1) - i];
          SelectedArea = a;
          SelectCount = i + 1;
          break;
        }
        newy -= CardOL;
      }
      SelectRect.x = Spots[a - 1].x;
      SelectRect.y = Spots[a - 1].y + (CardOL * TableauCounts[a - 7]) - (SelectCount * CardOL) + CardOL;
      SelectRect.w = CardWidth;
      SelectRect.h = CardHeight + (CardOL * SelectCount) - CardOL;
      DrawSelection();
    }
    else
    {
      // only 1 card on pile..
      SelectedCard = Tableau[a - 7][TableauCounts[a - 7] - 1];
      SelectedArea = a;
      SelectCount = 1;
      SelectRect.x = Spots[a - 1].x;
      SelectRect.y = Spots[a - 1].y + CardOL;
      SelectRect.w = Spots[a - 1].w;
      SelectRect.h = CardHeight; // Spots[a - 1].h;
      DrawSelection();
    }
  }
  else
  {
    // no cards on tableau, see if we have any hidden..
    if (HiddenCounts[a - 7] > 0)
    {
      // flip the hidden card..
      Tableau[a - 7][0] = HiddenTabs[a - 8][HiddenCounts[a - 7] - 1];
      TableauCounts[a - 7] = 1;
      HiddenTabs[a - 8][HiddenCounts[a - 7] - 1] = -1;
      HiddenCounts[a - 7]--;
      int newy = 0;// CardOL * TableauCounts[a - 7];
      Spots[a - 1].h = CardHeight + newy;
      newy = CardOL;
      if (HiddenCounts[a - 7] == 0)
      { // erase hidden placeholder..
        M5.Display.fillRect(Spots[a - 1].x, Spots[a - 1].y, CardWidth, CardHeight, TFT_DARKGREEN);
        M5.Display.drawRoundRect(Spots[a - 1].x, Spots[a - 1].y, CardWidth, CardHeight, 4, TFT_YELLOW);
      }
      M5.Display.pushImage(Spots[a - 1].x, Spots[a - 1].y + newy, CardWidth, CardHeight, card[Tableau[a - 7][TableauCounts[a - 7] - 1]]);
      M5.Display.drawRoundRect(Spots[a - 1].x, Spots[a - 1].y + newy, CardWidth, CardHeight, 4, TFT_YELLOW);
    }
  }
}

void CutDeck(int8_t *deck, int size){
  int8_t half[size / 2];
  memcpy(half,deck,sizeof(half));
  memcpy(deck,&deck[(size / 2)],sizeof(half));
  memcpy(&deck[(size / 2)],half,sizeof(half));
}

void shuffle(int8_t *array, int size)
{
  for (int i = size - 1; i > 0; i--)
  {
    int j = i + (rand() % (size - i));
    int8_t temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

void DrawSelection()
{
  if (SelectedCard > -1)
  {
    // selecting
    M5.Display.drawRoundRect(SelectRect.x + 1, SelectRect.y + 1, SelectRect.w - 3, SelectRect.h - 2, 4, TFT_GREEN);
    M5.Display.drawRoundRect(SelectRect.x + 2, SelectRect.y + 2, SelectRect.w - 4, SelectRect.h - 3, 4, TFT_GREEN);
    M5.Display.drawRoundRect(SelectRect.x, SelectRect.y, SelectRect.w, SelectRect.h, 4, TFT_GREEN);
  }
  else
  {
    M5.Display.drawRoundRect(SelectRect.x + 1, SelectRect.y + 1, SelectRect.w - 3, SelectRect.h - 2, 4, TFT_WHITE);
    M5.Display.drawRoundRect(SelectRect.x + 2, SelectRect.y + 2, SelectRect.w - 4, SelectRect.h - 3, 4, TFT_WHITE);
    M5.Display.drawRoundRect(SelectRect.x, SelectRect.y, SelectRect.w, SelectRect.h, 4, TFT_YELLOW);
    // unselecting
  }
}

void setup()
{
  M5.begin();
  M5.Display.setRotation(1);
  M5.Display.setSwapBytes(1);

  
  // init the deck..
  for (int i = 0; i < 52; i++)
    Deck[i] = i;
  // init random
  srand(time(0));
  // setup the table
  SetupTable();
}

void loop()
{
  now = millis();
  M5.update();
  m5::touch_detail_t touchDetail;

     touchDetail = M5.Touch.getDetail();

  if (touchDetail.isPressed())
  {
    if (now - lastTouch >= touchDebounce)
    { // got a touch..
      lastTouch = now;
      CheckScreenTouch(touchDetail.x, touchDetail.y);
    }
  }
  delay(1);
}

bool TouchArea(int px, int py, int x, int y, int w, int h)
{
  bool result = false;
  int x2 = x + w;
  int y2 = y + h;
  int touch_x = px;
  int touch_y = py;
  if ((touch_x >= x) && (touch_x <= x2) && (touch_y >= y) && (touch_y <= y2))
    result = true;
  return result;
}

void CheckScreenTouch(int tx, int ty)
{
  if (TouchArea(tx,ty,Button.x,Button.y,Button.w,Button.h)){
   SetupTable();
  } else
   {
  for (int i = 0; i < sizeof(Spots) / sizeof(Spots[0]); i++)
  {
    if (TouchArea(tx, ty, Spots[i].x, Spots[i].y, Spots[i].w, Spots[i].h))
    {
      HandleTouch(i, ty);
      break;
    }
  }
}
}

void HandleTouch(int spot, int y)
{

  switch (spot)
  {
  case 0:
    StockFlip();
    break;
  case 1:
    SelectWaste();
    break;
  case 2:
  case 3:
  case 4:
  case 5:
    if (SelectedCard > -1)
    {
      if (CheckFoundationMove(Spots[spot].area))
      {
        MoveToFoundations(Spots[spot].area);
      }
      else
      {
        // invalid move unselect the selection..
        SelectCount = 0;
        SelectedArea = 0;
        SelectedCard = -1;
        DrawSelection();
      }
    }
    else
    {
      SelectFoundation(Spots[spot].area);
    }
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
    if (SelectedCard > -1)
    {
      if (CheckTableauMove(Spots[spot].area))
      {
        MoveToTableau(Spots[spot].area);
      }
      else
      {
        // invalid move, unselect
        SelectCount = 0;
        SelectedArea = 0;
        SelectedCard = -1;
        DrawSelection();
      }
    }
    else
    {
      SelectTableau(Spots[spot].area, y);
    }
    break;
  default:
    break;
  }
}
