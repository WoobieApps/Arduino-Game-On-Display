#include <Adafruit_SH110X.h>
#include <stdlib.h>

#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define HEAD 0U
#define LEFT_BUTTON 2
#define RIGHT_BUTTON 3
#define BUTTON_TIMEOUT 500

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
    for(int i = 0; i <= 1 ; i++)
    {
      for(int j = 0; j <= 1 ; j++)
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
    SnakeSegments[HEAD+1].segment_init(62, 32);
    SnakeSegments[HEAD+2] = Segment();
    SnakeSegments[HEAD+2].segment_init(62, 32);
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
        SnakeSegments[HEAD].x -= 2;
      break;
      case RIGHT:
        SnakeSegments[HEAD].x += 2;
      break;
      case UP:
        SnakeSegments[HEAD].y -= 2;
      break;
      case DOWN:
        SnakeSegments[HEAD].y += 2;
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
  void check_collision()
  {
    for(unsigned int segment_id=HEAD+1; segment_id < SnakeLength; segment_id++)
    {
      if(SnakeSegments[segment_id].x == SnakeSegments[HEAD].x && SnakeSegments[segment_id].y == SnakeSegments[HEAD].y)
      {
        GameOver = true;
      }
    }
    if(SnakeSegments[HEAD].x < 2 || SnakeSegments[HEAD].x > 124 || SnakeSegments[HEAD].y < 2 || SnakeSegments[HEAD].y > 52)
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
void display_begin();

void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(0));
  
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);

  display.begin(i2c_Address, true);
  display.clearDisplay();
  display_begin();
  display.display();
  
  generate_treat();
  while(digitalRead(LEFT_BUTTON) == HIGH && digitalRead(RIGHT_BUTTON) == HIGH);
  display.clearDisplay();
  Snake.draw_Snake();
  draw_GUI(CurrentPoints);
  display.display();
  delay(2000);
}

void loop()
{
  display.clearDisplay();
  draw_GUI(CurrentPoints);
  draw_treat();
  change_directions();
  Snake.check_collision();
  while(GameOver){
    display.clearDisplay();
    display_game_over();
  }
  do_move();
  Snake.draw_Snake();
  check_treat();
  display.display();
}

void display_begin()
{
  int stringX = 46;
  display.drawChar(stringX-12,10,'S',1,1,2);
  display.drawChar(stringX,10,'N',1,1,2);
  display.drawChar(stringX+12,10,'A',1,1,2);
  display.drawChar(stringX+24,10,'K',1,1,2);
  display.drawChar(stringX+36,10,'E',1,1,2);
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

  display.drawChar(stringX+8,42,'S',1,1,1);
  display.drawChar(stringX+14,42,'C',1,1,1);
  display.drawChar(stringX+20,42,'O',1,1,1);
  display.drawChar(stringX+26,42,'R',1,1,1);
  display.drawChar(stringX+32,42,'E',1,1,1);

  draw_number(stringX+20, 54, CurrentPoints);

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
    display.drawPixel(x, 1, 1);
    display.drawPixel(x, 55, 1);
    display.drawPixel(x, 63, 1);
  }
  for(int y = 0; y < 64; y++)
  {
    display.drawPixel(0, y, 1);
    display.drawPixel(1, y, 1);
    display.drawPixel(126, y, 1);
    display.drawPixel(127, y, 1);
  }
  
  // scoreboard
  // S
  display.drawPixel(3,61,1);
  display.drawPixel(4,61,1);
  display.drawPixel(5,61,1);
  display.drawPixel(5,60,1);
  display.drawPixel(5,59,1);
  display.drawPixel(4,59,1);
  display.drawPixel(3,59,1);
  display.drawPixel(3,58,1);
  display.drawPixel(3,57,1);
  display.drawPixel(4,57,1); 
  display.drawPixel(5,57,1); 
  // C
  display.drawPixel(7,61,1);
  display.drawPixel(8,61,1);
  display.drawPixel(9,61,1);
  display.drawPixel(7,60,1);
  display.drawPixel(7,59,1);
  display.drawPixel(7,58,1);
  display.drawPixel(7,57,1);
  display.drawPixel(8,57,1);
  display.drawPixel(9,57,1);
  // O
  display.drawPixel(11,61,1);
  display.drawPixel(12,61,1);
  display.drawPixel(13,61,1);
  display.drawPixel(11,60,1);
  display.drawPixel(11,59,1);
  display.drawPixel(11,58,1);
  display.drawPixel(11,57,1);
  display.drawPixel(12,57,1);
  display.drawPixel(13,57,1);
  display.drawPixel(13,58,1);
  display.drawPixel(13,59,1);
  display.drawPixel(13,60,1);
  // R
  display.drawPixel(15,61,1);
  display.drawPixel(16,59,1);
  display.drawPixel(17,61,1);
  display.drawPixel(15,60,1);
  display.drawPixel(15,59,1);
  display.drawPixel(15,58,1);
  display.drawPixel(15,57,1);
  display.drawPixel(16,57,1);
  display.drawPixel(17,58,1);
  display.drawPixel(17,60,1);
  // E
  display.drawPixel(19,61,1);
  display.drawPixel(20,61,1);
  display.drawPixel(21,61,1);
  display.drawPixel(19,60,1);
  display.drawPixel(19,59,1);
  display.drawPixel(20,59,1);
  display.drawPixel(21,59,1);
  display.drawPixel(19,58,1);
  display.drawPixel(19,57,1);
  display.drawPixel(20,57,1);
  display.drawPixel(21,57,1);
  // :
  display.drawPixel(23,60,1);
  display.drawPixel(23,58,1);

  // SCORE
  draw_number(26,59,score);
  
}

void generate_treat()
{
  TreatCurrentX = (unsigned int)(random(0,30))*4+2;
  TreatCurrentY = (unsigned int)(random(0,12))*4+2;
}

void draw_treat()
{
  for(int i = 0; i <= 1 ; i++)
  {
    for(int j = 0; j <= 1 ; j++)
    {
      display.drawPixel(TreatCurrentX+i, TreatCurrentY+j, SH110X_WHITE);
    }
  }
}


