#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <time.h>
#include <stdlib.h>

#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define HEAD 0U
#define LEFT_BUTTON 2
#define RIGHT_BUTTON 3
#define BUTTON_TIMEOUT 600

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

enum Direction {LEFT, UP, RIGHT, DOWN};
bool GameOver = false;

struct Segment
{
  unsigned short int x;
  unsigned short int y;

  void segment_init(unsigned short int _x, unsigned short int _y)
  {
    x = _x;
    y = _y;
  }
  void draw_segment()
  {
    for(int i = -1; i <= 1 ; i++)
    {
      for(int j = -1; j <= 1 ; j++)
      {
        display.drawPixel(x+i, y+j, SH110X_WHITE);
      }
    }
  }
};

struct Snake
{
  private:
  Segment SnakeSegments[60];
  unsigned int SnakeLength = 3;
  public:
  Snake()
  {
    // Initialize first Snake Coordinates
    SnakeSegments[HEAD] = Segment();
    SnakeSegments[HEAD].segment_init(62, 30);
    SnakeSegments[HEAD+1] = Segment();
    SnakeSegments[HEAD+1].segment_init(62, 34);
    SnakeSegments[HEAD+2] = Segment();
    SnakeSegments[HEAD+2].segment_init(62, 38);
  }
  void draw_Snake()
  {
    for(unsigned int segment_id=HEAD; segment_id < SnakeLength; segment_id++)
    {
      SnakeSegments[segment_id].draw_segment();

    }
  }
  Segment get_head(){
    return SnakeSegments[HEAD];
  }

  void move(enum Direction direction, bool isEating)
  {
    unsigned int firstToBeChanged;
    if(isEating)
    {
      firstToBeChanged = SnakeLength-2;      
    }
    else 
    {
      firstToBeChanged = SnakeLength-1;
    }
    
    for(unsigned int segment_id=firstToBeChanged; segment_id >= HEAD+1; segment_id--)
    {
      SnakeSegments[segment_id].x = SnakeSegments[segment_id-1].x;
      SnakeSegments[segment_id].y = SnakeSegments[segment_id-1].y;
    }

    switch(direction)
    {
      case LEFT:
        SnakeSegments[HEAD].x -= 4;
      break;
      case RIGHT:
        SnakeSegments[HEAD].x += 4;
      break;
      case UP:
        SnakeSegments[HEAD].y -= 4;
      break;
      case DOWN:
        SnakeSegments[HEAD].y += 4;
      break;
      default:
        // do nothing
      break;
    }
  }
  void grow()
  {
    SnakeLength++;
    Segment last = SnakeSegments[SnakeLength-2];
    SnakeSegments[SnakeLength-1] = Segment();
    SnakeSegments[SnakeLength-1].segment_init(last.x, last.y);
  }
  bool check_collision()
  {
    for(unsigned int segment_id=HEAD+1; segment_id < SnakeLength; segment_id++)
    {
      if(SnakeSegments[segment_id].x == SnakeSegments[HEAD].x && SnakeSegments[segment_id].y == SnakeSegments[HEAD].y)
      {
        GameOver = true;
      }
    }
    if(SnakeSegments[HEAD].x < 2 || SnakeSegments[HEAD].x > 122 || SnakeSegments[HEAD].y < 2 || SnakeSegments[HEAD].y > 50)
    {
      GameOver = true;
    }
  }
};

Snake Snake;
Direction CurrentDirection = UP;
unsigned int TreatCurrentX = 255;
unsigned int TreatCurrentY = 255;
bool isTreatEaten = false;
int CurrentPoints = 0;

void draw_GUI(int score);
void draw_number(int x, int y, int number);
void draw_digit(int x, int y,int digit);
void generate_treat();
void draw_treat();
void change_directions();
void check_treat();
void do_move();
void display_game_over();

void setup()
{
  Serial.begin(9600);
  display.begin(i2c_Address, true);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);

  display.clearDisplay();
  display.display();
  randomSeed(analogRead(0));
  generate_treat();
}

void loop()
{
  display.clearDisplay();
  draw_GUI(CurrentPoints);
  draw_treat();
  change_directions();
  do_move();
  Snake.draw_Snake();
  check_treat();
  display.display();
  delay(100);
  while(GameOver){
    display.clearDisplay();
    display_game_over();
  }
}

void display_game_over()
{
  int stringX = 41;
  display.drawChar(stringX,10,'G',1,1,2);
  display.drawChar(stringX+12,10,'A',1,1,2);
  display.drawChar(stringX+24,10,'M',1,1,2);
  display.drawChar(stringX+36,10,'E',1,1,2);

  display.drawChar(stringX,26,'O',1,1,2);
  display.drawChar(stringX+12,26,'V',1,1,2);
  display.drawChar(stringX+24,26,'E',1,1,2);
  display.drawChar(stringX+36,26,'R',1,1,2);
  
  display.display();
}

void do_move(){
  if(isTreatEaten)
  {
    Snake.grow();
    Snake.move(CurrentDirection,true);
    isTreatEaten = false;
  }
  else
  {
    Snake.move(CurrentDirection,false);
  }
  Snake.check_collision();
}

void change_directions()
{
  bool changeDone = false;
  long long int time_before = millis();
  long long int time_now = millis();
  while(time_now-time_before < BUTTON_TIMEOUT)
  {
    if(digitalRead(LEFT_BUTTON) == LOW && !changeDone)
    {
      switch(CurrentDirection)
      {
        case LEFT:
          CurrentDirection = DOWN;
        break;
        case RIGHT:
          CurrentDirection = UP;
        break;
        case UP:
          CurrentDirection = LEFT;
        break;
        case DOWN:
          CurrentDirection = RIGHT;
        break;
        default:
          // do nothing
        break;
      }
      changeDone = true;
    }
    else if(digitalRead(RIGHT_BUTTON) == LOW && !changeDone)
    {
      switch(CurrentDirection)
      {
        case LEFT:
          CurrentDirection = UP;
        break;
        case RIGHT:
          CurrentDirection = DOWN;
        break;
        case UP:
          CurrentDirection = RIGHT;
        break;
        case DOWN:
          CurrentDirection = LEFT;
        break;
        default:
          // do nothing
        break;
      }
      changeDone = true;
    }
    time_now = millis();
  }
}

void check_treat()
{
  if ((abs((int)(Snake.get_head().x - TreatCurrentX)))==0 && (abs((int)(Snake.get_head().y - TreatCurrentY)))==0)
  {
    generate_treat();
    isTreatEaten = true;
    CurrentPoints++;
  }
}

void draw_number(int x, int y, int number)
{
  int singles = number%10;
  int tens = (number/10)%10;
  int hundreds = ((number/10)/10)%10;
  int thousands = (((number/10)/10)/10)%10;

  if(thousands != 0)
  {
    draw_digit(x,y,thousands);
    draw_digit(x+4,y,hundreds);
    draw_digit(x+8,y,tens);
    draw_digit(x+12,y,singles);
  }
  else if (hundreds!=0)
  {
    draw_digit(x,y,hundreds);
    draw_digit(x+4,y,tens);
    draw_digit(x+8,y,singles);
  }
  else if (tens!=0)
  {
    draw_digit(x,y,tens);
    draw_digit(x+4,y,singles);
  }
  else
  {
    draw_digit(x,y,singles);
  }
}

void draw_digit(int x, int y,int digit)
{
  switch(digit){
    case 1:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x+2, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
    break;
    case 2:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x+2, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x+1, y, 1);
      display.drawPixel(x, y, 1);
      display.drawPixel(x, y+1, 1);
      display.drawPixel(x, y+2, 1);
      display.drawPixel(x+1, y+2, 1);
      display.drawPixel(x+1, y-2, 1);
      display.drawPixel(x, y-2, 1);
    break;
    case 3:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x+2, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x+1, y, 1);
      display.drawPixel(x, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x, y+2, 1);
      display.drawPixel(x+1, y+2, 1);
      display.drawPixel(x+1, y-2, 1);
      display.drawPixel(x, y-2, 1);
    break;
    case 4:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x+2, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x+1, y, 1);
      display.drawPixel(x, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x, y-1, 1);
      display.drawPixel(x, y-2, 1);
    break;
    case 5:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x+1, y, 1);
      display.drawPixel(x, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x, y+2, 1);
      display.drawPixel(x+1, y+2, 1);
      display.drawPixel(x+1, y-2, 1);
      display.drawPixel(x, y-2, 1);
    break;
    case 6:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x+1, y, 1);
      display.drawPixel(x, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x, y+2, 1);
      display.drawPixel(x+1, y+2, 1);
      display.drawPixel(x+1, y-2, 1);
      display.drawPixel(x, y-2, 1);
      display.drawPixel(x, y+1, 1);
    break;
    case 7:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x+2, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x+1, y-2, 1);
      display.drawPixel(x, y-2, 1);
    break;
    case 8:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x+1, y, 1);
      display.drawPixel(x, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x, y+2, 1);
      display.drawPixel(x+1, y+2, 1);
      display.drawPixel(x+1, y-2, 1);
      display.drawPixel(x, y-2, 1);
      display.drawPixel(x, y+1, 1);
      display.drawPixel(x+2, y-1, 1);
    break;
    case 9:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x+1, y, 1);
      display.drawPixel(x, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x, y+2, 1);
      display.drawPixel(x+1, y+2, 1);
      display.drawPixel(x+1, y-2, 1);
      display.drawPixel(x, y-2, 1);
      display.drawPixel(x+2, y-1, 1);
    break;
    case 0:
      display.drawPixel(x+2, y, 1);
      display.drawPixel(x+2, y+2, 1);
      display.drawPixel(x, y-1, 1);
      display.drawPixel(x+2, y-2, 1);
      display.drawPixel(x, y, 1);
      display.drawPixel(x+2, y+1, 1);
      display.drawPixel(x, y+2, 1);
      display.drawPixel(x+1, y+2, 1);
      display.drawPixel(x+1, y-2, 1);
      display.drawPixel(x, y-2, 1);
      display.drawPixel(x, y+1, 1);
      display.drawPixel(x+2, y-1, 1);
    break;
  }
}

void draw_GUI(int score)
{
  for(int x = 0; x < 128; x++)
  {
    display.drawPixel(x, 0, 1);
    display.drawPixel(x, 54, 1);
    display.drawPixel(x, 55, 1);
    display.drawPixel(x, 63, 1);
  }
  for(int y = 0; y < 64; y++)
  {
    display.drawPixel(0, y, 1);
    display.drawPixel(127, y, 1);
  }
  
  // scoreboard
  // S
  display.drawPixel(2,61,1);
  display.drawPixel(3,61,1);
  display.drawPixel(4,61,1);
  display.drawPixel(4,60,1);
  display.drawPixel(4,59,1);
  display.drawPixel(3,59,1);
  display.drawPixel(2,59,1);
  display.drawPixel(2,58,1);
  display.drawPixel(2,57,1);
  display.drawPixel(3,57,1); 
  display.drawPixel(4,57,1); 
  // C
  display.drawPixel(6,61,1);
  display.drawPixel(7,61,1);
  display.drawPixel(8,61,1);
  display.drawPixel(6,60,1);
  display.drawPixel(6,59,1);
  display.drawPixel(6,58,1);
  display.drawPixel(6,57,1);
  display.drawPixel(7,57,1);
  display.drawPixel(8,57,1);
  // O
  display.drawPixel(10,61,1);
  display.drawPixel(11,61,1);
  display.drawPixel(12,61,1);
  display.drawPixel(10,60,1);
  display.drawPixel(10,59,1);
  display.drawPixel(10,58,1);
  display.drawPixel(10,57,1);
  display.drawPixel(11,57,1);
  display.drawPixel(12,57,1);
  display.drawPixel(12,58,1);
  display.drawPixel(12,59,1);
  display.drawPixel(12,60,1);
  // R
  display.drawPixel(14,61,1);
  display.drawPixel(15,59,1);
  display.drawPixel(16,61,1);
  display.drawPixel(14,60,1);
  display.drawPixel(14,59,1);
  display.drawPixel(14,58,1);
  display.drawPixel(14,57,1);
  display.drawPixel(15,57,1);
  display.drawPixel(16,58,1);
  display.drawPixel(16,60,1);
  // E
  display.drawPixel(18,61,1);
  display.drawPixel(19,61,1);
  display.drawPixel(20,61,1);
  display.drawPixel(18,60,1);
  display.drawPixel(18,59,1);
  display.drawPixel(19,59,1);
  display.drawPixel(20,59,1);
  display.drawPixel(18,58,1);
  display.drawPixel(18,57,1);
  display.drawPixel(19,57,1);
  display.drawPixel(20,57,1);
  // :
  display.drawPixel(22,60,1);
  display.drawPixel(22,58,1);

  // SCORE
  draw_number(25,59,score);
  
}

void generate_treat()
{
  TreatCurrentX = (unsigned int)(random(0,30))*4+2;
  TreatCurrentY = (unsigned int)(random(0,12))*4+2;
}

void draw_treat()
{
  for(int i = -1; i <= 1 ; i++)
  {
    for(int j = -1; j <= 1 ; j++)
    {
      display.drawPixel(TreatCurrentX+i, TreatCurrentY+j, SH110X_WHITE);
    }
  }
  display.drawPixel(TreatCurrentX,TreatCurrentY-2,1);
}


