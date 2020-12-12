// Frogger for reasonably powered MCU's. Need at least 48K RAM free and
// about 32Meg program memory
// Code written XTronical www.xtronical.com
// Original Arcade game copyright Konami 1981
// This code has an open source license, you can copy it, change it,
// for any purpose for commercial use or not. But these comments
// should be in you main file. The graphics are based upon but not the
// same as the originals. Copying these for any uses may be prohibited 
// by the Konami license - Author unsure.
// https://www.xtronical.com/5-flies-lady-frogs-scoring-sound/
// WIO Terminal port - dbzoo

// libraries for working with the screen
#include <TFT_eSPI.h>

// libraries for Frogger itself
#include "Graphics.h"
#include "Frogger.h"
#include "GraphicsData.h"
#include "Levels.h"

// Graphics sizes, the names refer to the original size and the actual sie in this version is the definition
#define G_WIDTH16 8                   // so this means any graphic that was originally 16 is now this value (8)
#define G_HEIGHT16 8

static TFT_eSPI tft = TFT_eSPI();
static TFT_eSPI_extended spr = TFT_eSPI_extended(&tft, TFT_WIDTH, TFT_HEIGHT);

Frogger_Class Frogger(spr);
//In the code we use Frogger.tft (its actually this spr)

void setup(void) {
  Serial.begin(115200);
  InitControls();
  tft.init();   // initialize TFT
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  spr.setColorDepth(16);
  Frogger.Frog=new Frog_Class(&Frogger.tft,FROG_X_START,FROG_Y_START,false);
  Frogger.LadyFrog=new Frog_Class(&Frogger.tft,-100,LADY_FROG_LANE_Y,true);
}

void InitControls()
{  
  pinMode(BUT_LEFT,INPUT_PULLUP);
  pinMode(BUT_RIGHT,INPUT_PULLUP);
  pinMode(BUT_UP,INPUT_PULLUP);
  pinMode(BUT_DOWN,INPUT_PULLUP);
  pinMode(BUT_PLAYER_1,INPUT_PULLUP);
  pinMode(BUT_PLAYER_2,INPUT_PULLUP);
}


void loop() 
{   
  if(Frogger.Frog->LivesLeft>0)
  {
    Frogger.Physics();
    UpdateDisplay();
  }
  else  
    AttractScreen();
}

void Frogger_Class::Physics()
{
  // rules of the game 
  if(PlayerActive) 
  {
    CheckInputs();
    CheckCollisions();
    ManageFly();
    ManageLadyFrog();
  }
  MoveObjects();
  TimerBar();
}




void Frogger_Class::ManageLadyFrog()
{
  if(LadyFrogActive==false)
  {
    // try to create a lady frog
    if(random(ODDS_OF_LADY_FROG)==1)        
    {
      // create one, lady frogs are created on the first log up from the middle safe area
      // and initially off screen, i.e. they don't just magically appear
      // Put on first off screen log on this row

      // Scanning the logs the fly can appear on, which are kept in a special list 
      // looking for the first that is off screen to out lady frog on
      uint8_t i=0;
      bool Found=false;
      Log_Class *TheLog;  
      while((i<LadyFrogIdx)&(Found==false))
      {
        TheLog=LadyFrogLogs[i];
        if(TheLog->x+TheLog->width<X_START)
        {
          // Found a log, put lady frog on it      
          Found=true;
          LadyFrogActive=true;
          Frogger.LadyFrog->SetAnimation(JUMP_UP,&Frogger);
          Frogger.LadyFrog->InitAnimationFrame(JUMP_UP,-1); 
          LadyFrogMoveTime=millis();
          LadyFrog->x=TheLog->x;
          LadyFrog->y=LADY_FROG_LANE_Y;
          LadyFrog->Velocity_H=TheLog->Velocity_H;
          LadyFrogLog=TheLog;
        }
        i++;
      }
    }
  }
  else
  {
    // Is she on froggers back
    if(LadyFrogOnFrogsBack)
    {
      LadyFrog->x=Frog->x;
      LadyFrog->y=Frog->y;
      return;
    }
    // Is she off screen    
    if(LadyFrog->x>SCREEN_WIDTH)
    {
      LadyFrogActive=false;
      LadyFrogLog=0;
    }
    else
    {
      // handle her on log movements
      if(millis()>LadyFrogMoveTime+DELAY_BETWEEN_LADY_FROG_MOVES)
      {
        // time for a new move
        LadyFrogMoveTime=millis();                        //Reset time
        // Now decide where to move
        if(LadyFrog->CurrentAnimationIdx==JUMP_UP)
        {
          // currently not moving, default up position
          // look to the left and see if we are at left of the log
          if(LadyFrog->x-2<LadyFrogLog->x)
          {
            // on left hand side of log, not moving, move frog to right            
            Frogger.LadyFrog->SetAnimation(JUMP_RIGHT,&Frogger);
            Frogger.LadyFrog->InitAnimationFrame(JUMP_RIGHT,0); 
          }
          else
          {
            if(LadyFrog->x+LadyFrog->width+2>LadyFrogLog->x+LadyFrogLog->width)
            {
              // on right hand side of log, not moving, move frog to right              
              Frogger.LadyFrog->SetAnimation(JUMP_LEFT,&Frogger);
              Frogger.LadyFrog->InitAnimationFrame(JUMP_LEFT,0); 
            }          
          }
        }
        else
        {
          // already moving , is it at the end of a log? If so go to default up position          
          if((LadyFrog->x+LadyFrog->width+2>LadyFrogLog->x+LadyFrogLog->width)|(LadyFrog->x-2<LadyFrogLog->x))
          {
              // Yes at right of log, reset to up
            Frogger.LadyFrog->SetAnimation(JUMP_UP,&Frogger);
            Frogger.LadyFrog->InitAnimationFrame(JUMP_UP,-1); 
          } 
          else
          {
            // must be jumping, set next jump
            if(Frogger.LadyFrog->CurrentAnimationIdx==JUMP_LEFT)
            {              
              Frogger.LadyFrog->SetAnimation(JUMP_LEFT,&Frogger);
              Frogger.LadyFrog->InitAnimationFrame(JUMP_LEFT,0);               
            }
            else
            {          
              Frogger.LadyFrog->SetAnimation(JUMP_RIGHT,&Frogger);
              Frogger.LadyFrog->InitAnimationFrame(JUMP_RIGHT,0);               
            }
          }
        }
      }
    }
  }
}



void ManageFly()
{
  // a 1 in ODDS_OF_FLY chance of a fly appearing
  if(Frogger.FlyIdx>4)                                     // No current fly, try to generate one
  {
    if(random(ODDS_OF_FLY)==1)        
    {
      while(Frogger.FlyIdx>4)                             // repeat until a free home is found
      {
        Frogger.FlyIdx=random(5);                          // which frog home for it to appear in
        if(Frogger.FrogHome[Frogger.FlyIdx]==true)
          Frogger.FlyIdx=5;                               // not free,frog is home, try again
      }
      // Set current fly time start
      Frogger.FlyTime=millis();
    }
  }
  else
  {
    // there is a current fly, should it be taken off the screen?, check the timer
    if(millis()>Frogger.FlyTime+TIME_FLY_ACTIVE)      // time is up, cancel fly
      Frogger.FlyIdx=5;
  }
}



void TimerBar()
{
  // calculate time left and timer bar x pos
  Frogger.TimeElapsed=millis()-Frogger.Frog->TimeStart;
  Frogger.TimeLeft=60-(uint8_t(float(Frogger.TimeElapsed)/523));      // Divide by 533 as there are 0.533s per bar digit
  // If ran out of time and frog not already dying then kill frog
  if((Frogger.TimeLeft<=0)&(Frogger.Frog->CurrentAnimationIdx!=DROWNING_DEATH)&(Frogger.Frog->CurrentAnimationIdx!=COLLISION_DEATH))
  {    
    Frogger.Frog->SetAnimation(COLLISION_DEATH,&Frogger);    
  }
}


/////////////////////////////////////////////////////////////////////////////////////
// Colision routines

void CheckCollisions()
{
  // go through all objects and check for collision as long as not currentyly displaying death animation
  
  Frogger.Frog->OnFloatingObject=false;
  if((Frogger.Frog->CurrentAnimationIdx!=DROWNING_DEATH)&(Frogger.Frog->CurrentAnimationIdx!=COLLISION_DEATH))
  {    
    MovingObject_Class* GameObject;
    if(Frogger.GameObjectListStart!=0)
    {
      GameObject=Frogger.GameObjectListStart;
      while((GameObject!=0)&(Frogger.PlayerActive))
      {
        GameObject->Collision(Frogger.Frog,&Frogger);         // checks and handles any collision
        GameObject=GameObject->NextObj;
      }
    }
    //if we get here nad not active then traffic death
    if(Frogger.PlayerActive==false)           
    
    if((Frogger.InRiver()) &(Frogger.Frog->OnFloatingObject==false))        // In river area and not on a floating object
    {      
        if(Frogger.Frog->CurrentFrame==-1)                            // if we are jumping not jumping then death
        {
          Frogger.Frog->SetAnimation(DROWNING_DEATH,&Frogger); 
          Frogger.LadyFrogActive=false;
          Frogger.LadyFrogOnFrogsBack=false;
        }
    }
  
    // Has frog made it home, unlikely if I'm playing but we'll check anyways
    if(Frogger.PlayerActive)
    {
      if(Frogger.Frog->y<HEDGE_Y_START+(HEDGE_PIECE_HEIGHT*NUMBER_VERTICAL_HEDGE_PIECES))
      {
        // we are high enough to either be in a hedge or in a home, let's check
        // got through each home coordinate
        for(int i=0;i<5;i++)
        {
          if((Frogger.Frog->x>Frogger.Homes[i]) & (Frogger.Frog->x+Frogger.Frog->width<Frogger.Homes[i]+HEDGE_PIECE_WIDTH*3))
          {
            Frogger.GotHome(i);
            return;
          }
        }   
        // has not got home, must be in the hedge
          Frogger.Frog->SetAnimation(COLLISION_DEATH,&Frogger);
      }
      // Has frogger landed on lady frog
      if((Frogger.LadyFrogActive)&(Frogger.LadyFrogOnFrogsBack==false))
      {
        if(Frogger.LadyFrog->CollidedWith(Frogger.Frog))         // checks and handles any collision
        {
          Frogger.LadyFrogOnFrogsBack=true;          
          Frogger.LadyFrog->SetAnimation(JUMP_UP,&Frogger);       // set to up animation
          Frogger.LadyFrog->InitAnimationFrame(JUMP_UP,-1); 
        }
      }
    }
  }
}


bool Frogger_Class::InRiver()
{
  // returns truw if in river area of game
  
  return ((Frogger.Frog->y<RIVER_END)&(Frogger.Frog->y>HEDGE_Y_START+(HEDGE_PIECE_HEIGHT*NUMBER_VERTICAL_HEDGE_PIECES)));
}


// End of collision routines
///////////////////////////////////////////////////////////////////////////////////




// Button input
///////////////////////////////////////////////////////////////////////////////////

void CheckInputs()
{
  // Check player inputs, in Frogger you cannot hold down a movement direction to get repeated jumps in the direction
  // For each jump the corrosponding button must be releases and re-pressed. Hence we have some button status's to
  // record button states.

  // only check for an input if frog is currently not moving, flagged by current frame index of current animation being -1
  if(Frogger.Frog->CurrentFrame==-1)
  {
    if((digitalRead(BUT_UP)==0)&(Frogger.UpPressed==false))  
    {
      Frogger.UpPressed=true;
      Frogger.Frog->SetAnimation(JUMP_UP,&Frogger);      
      // scoring
      Frogger.Frog->CurrentHopPos++;
      if(Frogger.Frog->CurrentHopPos>Frogger.Frog->FurthestHopPos)
      {
        Frogger.Frog->FurthestHopPos=Frogger.Frog->CurrentHopPos;
        if(Frogger.Frog->CurrentHopPos!=6)                        // no points for path in middle
          IncScore(10);
      }
    }
    
    if((digitalRead(BUT_DOWN)==0)&(Frogger.DownPressed==false))  
    {
      Frogger.DownPressed=true;
      Frogger.Frog->SetAnimation(JUMP_DOWN,&Frogger);      
      Frogger.Frog->CurrentHopPos--;
    }
    
    if((digitalRead(BUT_LEFT)==0)&(Frogger.LeftPressed==false))  
    {
      Frogger.LeftPressed=true;
      Frogger.Frog->SetAnimation(JUMP_LEFT,&Frogger);      
    }
    
    if((digitalRead(BUT_RIGHT)==0)&(Frogger.RightPressed==false))  
    {
      Frogger.RightPressed=true;
      Frogger.Frog->SetAnimation(JUMP_RIGHT,&Frogger);      
    }
  }
  // check for released buttons and reset flags as required
  if((digitalRead(BUT_UP)==1)&(Frogger.UpPressed))  
    Frogger.UpPressed=false;
    
  if((digitalRead(BUT_DOWN)==1)&(Frogger.DownPressed))  
    Frogger.DownPressed=false;  
    
  if((digitalRead(BUT_LEFT)==1)&(Frogger.LeftPressed))  
    Frogger.LeftPressed=false;  
    
  if((digitalRead(BUT_RIGHT)==1)&(Frogger.RightPressed))  
    Frogger.RightPressed=false;
}



// end button inputs
/////////////////////////////////////////////////////////////////////////////////////////////




void IncScore(uint16_t Amount)
{
  Frogger.Score+=Amount;
  if(Frogger.Score>Frogger.HiScore)
    Frogger.HiScore=Frogger.Score;
}


void MoveObjects()
{
  Frogger.Frog->Move(&Frogger);
  Frogger.LadyFrog->Move(&Frogger);
  MovingObject_Class* GameObject;
  if(Frogger.GameObjectListStart!=0)
  {
    GameObject=Frogger.GameObjectListStart;
    while(GameObject!=0)
    {
      GameObject->x+=GameObject->Velocity_H;
      if(GameObject->Velocity_H>0)  // moving right
      {
        if(GameObject->x>SCREEN_WIDTH+GameObject->DistanceToTravelOffScreen)
          GameObject->x=-GameObject->width;
      }
      else                          // moving left
      {        
        if(GameObject->x+GameObject->width<-DISTANCE_TO_TRAVEL_OFF_SCREEN)
          GameObject->x=SCREEN_WIDTH;
      }
      GameObject=GameObject->NextObj;
    }
  }

  // Special events
  if(Frogger.AllHome)
    AllHomeSequence();
}



void AllHomeSequence()
{
  // controls the sequence that happens when all frogs are home
  // In the original arcade the home frogs stick out their tongues and wink one by one from left to right
  // On small screens you just won't notice this, even in the arcade it was easy to miss, so I will drift a 
  // little from the original by just removing frogs from left to right

  if(millis()>Frogger.AllHomeTimer+ALL_HOME_ANIMATION_INTERVAL)
  {
    // erase a frog
    uint8_t FrogIdx=0;
    bool AllErased=true;
    while(FrogIdx<5)
    {
      if(Frogger.FrogHome[FrogIdx]==true)
      {
        Frogger.FrogHome[FrogIdx]=false;
        FrogIdx=5;                // gets out of loop
        AllErased=false;
      }
      FrogIdx++;
    }
    // Reset frog animation counter
    Frogger.AllHomeTimer=millis();
    if(AllErased)
    {
      // Set up next level
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Display code
////////////////////////////////////////////////////////////////////////////////////////////////////
void UpdateDisplay()
{
  // display graphics
  DisplayBackground();
  DisplayObjects();
  Frogger.Frog->Draw(X_START);              // Draw the frog
  if(Frogger.LadyFrogActive)
    Frogger.LadyFrog->Draw(X_START);          // Draw the frog
  
  // clear sides where moving objects plotted but shouldn't show
  Frogger.tft.fillRect(0,0,X_START,TFT_HEIGHT,TFT_BLACK);       
  Frogger.tft.fillRect(SCREEN_END_X+1,0,X_START,TFT_HEIGHT,TFT_BLACK);   // X_START FOR WIDTH AS SHOULD BE SAME AT RIGHT SIDE    

  DrawHomeFrogs();
  DrawFly();
  Frogger.tft.pushSprite(96,56); // Center 128,128 sprite
}


void DrawFly()
{
  if(Frogger.FlyIdx<5)
      Frogger.tft.drawRGBBitmap(X_START+Frogger.Homes[Frogger.FlyIdx]+1,HEDGE_Y_START+HEDGE_PIECE_HEIGHT,Fly,6,5); 
}

void DrawHomeFrogs()
{
  
  for(int i=0;i<5;i++)
  {
    if(Frogger.FrogHome[i])
      Frogger.tft.drawRGBBitmap(X_START+Frogger.Homes[i],HEDGE_Y_START+HEDGE_PIECE_HEIGHT-1,HomeFrog,8,8);    
  }
}

void DisplayObjects()
{
  MovingObject_Class* GameObject;
  if(Frogger.GameObjectListStart!=0)
  {
    GameObject=Frogger.GameObjectListStart;
    while(GameObject!=0)
    {
      GameObject->Draw(X_START);
      GameObject=GameObject->NextObj;
    }
  }
}


void DisplayBackground()
{
  // display graphics that frogger does not interact with directly, such as road, river, hedges, paths
  
  Frogger.tft.fillRect(X_START,0,SCREEN_END_X,RIVER_END+1,TFT_BLUE);                   // draw river
  Frogger.tft.fillRect(X_START,RIVER_END+8,SCREEN_END_X,SCREEN_END_X,TFT_BLACK);       // draw road
  DrawPavements();
  DrawHedges();
  Stats();
}

void Stats()
{
  // display scores, lives etc.
  int i;
  uint32_t Colour;
  
  // Hi score
  char charBuf[8]="0000000";
//  String str = String(Frogger.HiScore);
  String str = String(Frogger.TimeLeft);
  DrawText(&Frogger.tft,"HI SCORE",60,0,TFT_WHITE);
  str.toCharArray(charBuf+(7-str.length()), 8);
  DrawText(&Frogger.tft,charBuf,64,5,TFT_RED);
  
  // score
  str = String(Frogger.Score);
  DrawText(&Frogger.tft,"SCORE",X_START,0,TFT_WHITE);
  for (i = 0; i < 8; i++)      // clear buffer back to 0's  
    charBuf[i] = '0';  
  str.toCharArray(charBuf+(7-str.length()), 8);
  DrawText(&Frogger.tft,charBuf,X_START,5,TFT_RED);

  //lives left
  for(i=Frogger.Frog->LivesLeft-2;i>=0;i--)    
    Frogger.tft.drawRGBBitmap(X_START+i*5+1,121,FrogLives,4,4);

  // time left  
  if(Frogger.TimeLeft<=10)
    Colour=TFT_RED;
  else
    Colour=TFT_GREEN;
  Frogger.tft.fillRect((SCREEN_END_X-16)-Frogger.TimeLeft,124,Frogger.TimeLeft,4,Colour);       // draw time left
  DrawText(&Frogger.tft,"TIME",SCREEN_END_X-16,124,TFT_YELLOW);
}


void DrawPavements()
{
  //draw pavements, 16 across
  for(int i=0;i<SCREEN_WIDTH/G_WIDTH16;i++)  
  {
    Frogger.tft.drawRGBBitmap(X_START+i*G_WIDTH16,RIVER_END+1,Pavement,G_WIDTH16,G_HEIGHT16);
    Frogger.tft.drawRGBBitmap(X_START+i*G_WIDTH16,BOTTOM_PAVEMENT_Y,Pavement,G_WIDTH16,G_HEIGHT16);
  }
}


void DrawHedges()
{
  // draw hedges
  int i;
  
  i=0;
  int y=HEDGE_Y_START;
  int x=X_START;
  while(HedgeData[i]!=0)
  {
    DrawHedgePiece(HedgeData[i],x,y);
    i++;
    x+=HEDGE_PIECE_WIDTH;
    if(x>=SCREEN_END_X)
    {
      y+=4;
      x=X_START;
    }
  }
}


void DrawHedgePiece(char Piece,int x,int y)
{
  // draws a single hedge piece at x,y, note code 9 is a blank area
  switch(Piece)
  {
    case 1: Frogger.tft.drawRGBBitmap(x,y,HedgeTopLeft,HEDGE_PIECE_WIDTH,HEDGE_PIECE_HEIGHT);break;
    case 2: Frogger.tft.drawRGBBitmap(x,y,HedgeTop,HEDGE_PIECE_WIDTH,HEDGE_PIECE_HEIGHT-1);break;
    case 3: Frogger.tft.drawRGBBitmap(x,y,HedgeTopRight,HEDGE_PIECE_WIDTH,HEDGE_PIECE_HEIGHT);break;
    case 4: Frogger.tft.drawRGBBitmap(x,y,HedgeMiddle,HEDGE_PIECE_WIDTH,HEDGE_PIECE_HEIGHT);break;
    case 5: Frogger.tft.drawRGBBitmap(x,y,HedgeLeftSide,HEDGE_PIECE_WIDTH-1,HEDGE_PIECE_HEIGHT);break;
    case 6: Frogger.tft.drawRGBBitmap(x+1,y,HedgeRightSide,HEDGE_PIECE_WIDTH-1,HEDGE_PIECE_HEIGHT);break;
    case 7: Frogger.tft.drawRGBBitmap(x,y,HedgeBottomLeft,HEDGE_PIECE_WIDTH-1,HEDGE_PIECE_HEIGHT-1);break;
    case 8: Frogger.tft.drawRGBBitmap(x+1,y,HedgeBottomRight,HEDGE_PIECE_WIDTH-1,HEDGE_PIECE_HEIGHT-1);break;
  }
}


void AttractScreen()
{
  Frogger.tft.fillScreen(TFT_BLACK);
  Frogger.tft.setTextColor(TFT_WHITE);
  Frogger.tft.drawString("Press Start", 10, 64);
  if((digitalRead(BUT_PLAYER_1)==0)|(digitalRead(BUT_PLAYER_2)==0))  
  {    
    Frogger.StartNewGame();
  }
  Frogger.tft.pushSprite(96,56); // Center 128,128 sprite
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frogger class functions 


void Frogger_Class::GotHome(uint8_t HomeIndex)
{
  uint16_t TotalScore=50;
  // place in home frog
  FrogHome[HomeIndex]=true;
  // Add score, which is 50pts for getting home plus 10 pts for every game beat left on the clock
  // If you take a female frog home that's 200pts extra and if you catch a fly in the "home" another 200pts
  
  if(FlyIdx==HomeIndex)  {
    TotalScore+=200;
    FlyIdx=5;             // remove fly at this location
  }
  if(LadyFrogOnFrogsBack)
  {
    TotalScore+=200;
    LadyFrogActive=false;
    LadyFrogOnFrogsBack=false;
  }
    
  IncScore(TotalScore+(Frogger.TimeLeft*10));
  
  // Are all frogs home, if so flag level end animation and disable player motion
  AllHome=true;
  for(int i=0;i<5;i++)
  {
    if(FrogHome[i]==false)
      AllHome=false;
  }
  if(AllHome)
  {
    PlayerActive=false;
    AllHomeTimer=millis();      // Set to now time and later we can use this to see how much time has elapsed
  }
  
  // return player to start
  Frog->SetHomePos(this);
}

void Frogger_Class::StartNewGame()
{
  //Clear any frogs left in homes
  for(int i=0;i<5;i++)
    FrogHome[i]=false;
  Frogger.GameInPlay=true;
  Frogger.PlayerActive=true;
  Frogger.Frog->LivesLeft=START_LIVES;
  Frogger.Level=1;
  NextLevel();
  Frogger.Frog->SetHomePos(&Frogger);
}

void Frogger_Class::NextLevel()
{
  FlyIdx=5;
  InitLevel(&Frogger,Level);
  
}

// End frogger class functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
