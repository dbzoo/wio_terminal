// classes and routines for frogger graphics
#include "Graphics.h"
#include "GraphicsData.h"
#include "Frogger.h"
#include "WIO_TFT.h"

BaseObject_Class::BaseObject_Class(TFT_eSPI_extended *tft, const unsigned short* gfx, int16_t width, int16_t height, float x, int16_t y)
{
  this->x = x;
  this->y = y;
  this->width = width;
  this->height = height;
  gfxData = gfx;
  this->tft = tft;
}


// Base Object routines

void BaseObject_Class::Draw(uint8_t OffsetX)
{
  if(Mask)
    drawRGBBitmap(tft,x+OffsetX,y,gfxData,width,height);
  else
    tft->drawRGBBitmap(x+OffsetX, y, gfxData, width, height);
}

bool BaseObject_Class::CollidedWith(MovingObject_Class *Object)
{
  return ((x+width>Object->x)&(x<Object->x+Object->width)&(y+height>Object->y)&(y<Object->y+Object->height));
}


bool BaseObject_Class::Collision(MovingObject_Class *Object,Frogger_Class *Frogger)
{
  // returns true if object is in collsion with this object, only used for collision objects, floating objects handle this temselves
  bool Collided=CollidedWith(Object);
  if(Collided)
  {
    Frogger->Frog->SetAnimation(COLLISION_DEATH,Frogger);
    Frogger->LadyFrogActive=false;
    Frogger->LadyFrogOnFrogsBack=false;
  }
  return Collided;
}



    

// Moving object class 

MovingObject_Class::MovingObject_Class(TFT_eSPI_extended *tft,const unsigned short* gfx,int16_t width,int16_t height,float x, int16_t y,float Velocity):BaseObject_Class(tft,gfx,width,height,x,y)
{
  Init(tft,gfx,width,height,x,y,Velocity,DISTANCE_TO_TRAVEL_OFF_SCREEN);  
}


MovingObject_Class::MovingObject_Class(TFT_eSPI_extended *tft,const unsigned short* gfx,int16_t width,int16_t height,float x, int16_t y,float Velocity,uint8_t DistOffScreen):BaseObject_Class(tft,gfx,width,height,x,y)
{
  Init(tft,gfx,width,height,x,y,Velocity,DistOffScreen);
}


void MovingObject_Class::Init(TFT_eSPI_extended *tft,const unsigned short* gfx,int16_t width,int16_t height,float x, int16_t y,float Velocity,uint8_t DistOffScreen)
{  
  this->Velocity_H=Velocity;
  this->PreviousObj=0;
  this->NextObj=0;
  DistanceToTravelOffScreen=DistOffScreen;
}

void MovingObject_Class::DecrementLives(Frogger_Class *Frogger)
{
  // does absolutly nothing
}


void MovingObject_Class::SetAnimation(uint8_t Index,Frogger_Class *Frogger)
{
  CurrentAnimationIdx=Index;
  CurrentFrame=0;
  AnimationCounter=Animation[CurrentAnimationIdx].Frame[CurrentFrame].ClicksToPlayFor;    // Length of time to keep this frame active.
  InitAnimationFrame(CurrentAnimationIdx,CurrentFrame);
  // if setting to a death animation, disable player
  if((CurrentAnimationIdx==DROWNING_DEATH)|(CurrentAnimationIdx==COLLISION_DEATH))
  {
    Frogger->PlayerActive=false;
    // MOVE frog y pos to nearestr whole block of 8 pixels
    y=y & 0xfff8;
  }
}


void MovingObject_Class::InitAnimationFrame(uint8_t AnimationIdx,uint8_t AnimationFrame)
{
  // sets up the frame for the animation and frame passed in
  // if frame is -1 then set to default frame gfx an animation is deemed ended
  CurrentFrame=AnimationFrame;
  if(AnimationFrame==-1) {
    AnimationFrame=Animation[AnimationIdx].DefaultFrame;
    JumpingInXOrd=false;
  }
  else
  {
    // if jumping in x ord set flag
    if((AnimationIdx==JUMP_LEFT)|(AnimationIdx==JUMP_RIGHT))
      JumpingInXOrd=true;
    else
      JumpingInXOrd=false;
  }
  gfxData=Animation[AnimationIdx].Frame[AnimationFrame].Gfx;
  width=Animation[AnimationIdx].Frame[AnimationFrame].width;
  height=Animation[AnimationIdx].Frame[AnimationFrame].height;
  AnimationCounter=Animation[AnimationIdx].Frame[AnimationFrame].ClicksToPlayFor;
  
}


// floating object class

FloatingObject_Class::FloatingObject_Class(TFT_eSPI_extended *tft,int16_t width,int16_t height,float x, int16_t y,float Velocity,uint8_t DistOffScreen):MovingObject_Class(tft,0,width,height,x,y,DistOffScreen)
{
  Init(tft,0,width,height,x,y,Velocity,DistOffScreen);  
}



bool FloatingObject_Class::Collision(MovingObject_Class *Object,Frogger_Class *Frogger)
{
  
  // collisions with floating objects result in the object travelling with this floating object (FO)
  // we simply alter the x ord of the object so that it travels with this FO
  // If its the first collision, i.e. just landed on the floating object then we store the objects
  // relative position on the floating object. We then maintain this relative position on the FO
  // as it moves. This relative position can only be changed if the object itself moves in either X or Y
  // Relative positions of NOT_FLOATING_ON_ANYTHING mean the object is not currently on a floating object
  // returns true if on a log (collision)


  if(Frogger->InRiver())
  {
    if((x+width>Object->x)&(x<Object->x+Object->width)&(y+height>Object->y)&(y<Object->y+Object->height))         // on floating object?
    { // yes
      Object->OnFloatingObject=true;
      if(gfxData==0)                                      // is object above water , i.e. graphic showing
          Object->SetAnimation(DROWNING_DEATH,Frogger);           // No lose life
      else
      {
        // has it just landed on floating object
        if(Object->CurrentFrame==-1)  // not currently on anything so freshly landed
          Object->Velocity_H=Velocity_H;
      }
    }
  }
  else
    Object->Velocity_H=0;        // denotes not on a log, obviously vannot be if not in river
  return false;

}

// log class


Log_Class::Log_Class(TFT_eSPI_extended *tft,float x, int16_t y,float Velocity,uint8_t Length,uint8_t DistOffScreen):FloatingObject_Class(tft,0,0,x, y,Velocity,DistOffScreen)
{
  this->Length=Length;
  this->width=LogLeftWidth+(Length*LogMiddleWidth)+LogRightWidth;
  this->height=LogLeftHeight;
  gfxData=LogLeft;                                                    // need something in here else collision presumes no image and would kill frog.
  ClassName="Log_Class";
}

void Log_Class::Draw(uint8_t OffsetX)
{
  tft->drawRGBBitmap(x+OffsetX, y, LogLeft, LogLeftWidth, LogLeftHeight);
  for(int i=0;i<Length;i++)
    tft->drawRGBBitmap(x+OffsetX+(LogLeftWidth)+(i*LogMiddleWidth), y, LogMiddle, LogMiddleWidth, LogMiddleHeight);
  tft->drawRGBBitmap(x+OffsetX+(LogLeftWidth)+(Length*LogMiddleWidth), y, LogRight, LogRightWidth, LogRightHeight);
}



// turtle class

Turtle_Class::Turtle_Class(TFT_eSPI_extended *tft,float x, int16_t y,float Velocity,bool Diving,uint8_t NumTurtlesPerGroup):FloatingObject_Class(tft,0,0,x, y,Velocity,DISTANCE_TO_TRAVEL_OFF_SCREEN)
{
  this->Diving=Diving;
  this->NumTurtlesPerGroup=NumTurtlesPerGroup;
  Frame=0;
  width=TurtleWidth*NumTurtlesPerGroup;
  height=TurtleHeight;
  AnimationSpeed=TURTLE_ANIMATION_SPEED;
  ClassName="Turtle_Class";                         // used whilst debugging
}


void Turtle_Class::Draw(uint8_t OffsetX)
{
  
  uint8_t LastFrame;

  if(!AnimationSpeed--)  
  {
    if(Diving)
      LastFrame=8;
    else
      LastFrame=3;
    Frame++;
    
    if(Frame==LastFrame)
      Frame=0;
    if(Frame<3)       // normal swimming frames run at different speed than diving
      AnimationSpeed=TURTLE_ANIMATION_SPEED;
    else
      AnimationSpeed=TURTLE_DIVING_ANIMATION_SPEED;
  }
  
  switch(Frame)
  {
    case 0: gfxData=Turtle1;break;
    case 1: gfxData=Turtle2;break;
    case 2: gfxData=Turtle3;break;
    case 3: gfxData=TurtleDiving1;break;
    case 4: gfxData=TurtleDiving2;break;
    case 5: gfxData=0;break;
    case 6: gfxData=TurtleDiving2;break;
    case 7: gfxData=TurtleDiving1;break;
  }

  if(Frame!=5)  // 5 is fully submerged, no graphic
  {
    for(int i=0;i<NumTurtlesPerGroup;i++)
    {
      tft->drawRGBBitmap(x+OffsetX+(i*TurtleWidth), y, gfxData, TurtleWidth, TurtleHeight);
    }
  }
}


//Frog Class
  
Frog_Class::Frog_Class(TFT_eSPI_extended *tft,float x, int16_t y,bool LadyFrog):MovingObject_Class(tft,0,0,0,x,y)
{
  const unsigned short *Up,*Up2,*Up3,*Down,*Down2,*Down3,*Left,*Left2,*Left3,*Right,*Right2,*Right3;
  
  LivesLeft=0;
  this->LadyFrog=LadyFrog;
  if(LadyFrog)
  {
    // Lady frog does not have all animation or use them, so unused ones set to normal frogger ones
    // doesn't matter , they'll never be seen
    Up=LadyFrog_Up;Up2=Frog_Up2;Up3=Frog_Up3;
    Down=Frog_Down;Down2=Frog_Down2;Down3=Frog_Down3;
    Left=LadyFrog_Left;Left2=LadyFrog_Left2;Left3=LadyFrog_Left3;
    Right=LadyFrog_Right;Right2=LadyFrog_Right2;Right3=LadyFrog_Right3;
  }
  else
  {
    Up=Frog_Up;Up2=Frog_Up2;Up3=Frog_Up3;
    Down=Frog_Down;Down2=Frog_Down2;Down3=Frog_Down3;
    Left=Frog_Left;Left2=Frog_Left2;Left3=Frog_Left3;
    Right=Frog_Right;Right2=Frog_Right2;Right3=Frog_Right3;
  }
  gfxData=Up;
  width=FrogUpWidth;
  height=FrogUpHeight;
  Mask=true;
  Animation[JUMP_UP].AddFrame(0,Up2,FrogUp2Width,FrogUp2Height,0,-3,JUMP_SPEED);
  Animation[JUMP_UP].AddFrame(1,Up3,FrogUp3Width,FrogUp3Height,0,-4,JUMP_SPEED);
  Animation[JUMP_UP].AddFrame(2,Up,FrogUpWidth,FrogUpHeight,0,-1,JUMP_SPEED);
  
  Animation[JUMP_RIGHT].AddFrame(0,Right2,FrogRight2Width,FrogRight2Height,3,0,JUMP_SPEED);
  Animation[JUMP_RIGHT].AddFrame(1,Right3,FrogRight3Width,FrogRight3Height,4,0,JUMP_SPEED);
  Animation[JUMP_RIGHT].AddFrame(2,Right,FrogRightWidth,FrogRightHeight,1,0,JUMP_SPEED);
  
  Animation[JUMP_DOWN].AddFrame(0,Down2,FrogDown2Width,FrogDown2Height,0,3,JUMP_SPEED);
  Animation[JUMP_DOWN].AddFrame(1,Down3,FrogDown3Width,FrogDown3Height,0,4,JUMP_SPEED);
  Animation[JUMP_DOWN].AddFrame(2,Down,FrogDownWidth,FrogDownHeight,0,1,JUMP_SPEED);
  
  Animation[JUMP_LEFT].AddFrame(0,Left2,FrogLeft2Width,FrogLeft2Height,-3,0,JUMP_SPEED);
  Animation[JUMP_LEFT].AddFrame(1,Left3,FrogLeft3Width,FrogLeft3Height,-4,0,JUMP_SPEED);
  Animation[JUMP_LEFT].AddFrame(2,Left,FrogLeftWidth,FrogLeftHeight,-1,0,JUMP_SPEED);

  // ADD DEATH ANIMATIONS
  // Drowning
  Animation[DROWNING_DEATH].AddFrame(0,DrowningDeath0,DeathWidth,DeathHeight,0,0,6);
  Animation[DROWNING_DEATH].AddFrame(1,DrowningDeath1,DeathWidth,DeathHeight,0,0,4);
  Animation[DROWNING_DEATH].AddFrame(2,DrowningDeath2,DeathWidth,DeathHeight,0,0,6);
  Animation[DROWNING_DEATH].AddFrame(3,DrowningDeath3,DeathWidth,DeathHeight,0,0,12);

  // collision
  Animation[COLLISION_DEATH].AddFrame(0,CollisionDeath0,DeathWidth,DeathHeight,0,0,6);
  Animation[COLLISION_DEATH].AddFrame(1,CollisionDeath1,DeathWidth,DeathHeight,0,0,4);
  Animation[COLLISION_DEATH].AddFrame(2,CollisionDeath2,DeathWidth,DeathHeight,0,0,6);
  Animation[COLLISION_DEATH].AddFrame(3,DrowningDeath3,DeathWidth,DeathHeight,0,0,12);
  
}

void Frog_Class::DecrementLives(Frogger_Class *Frogger)
{
  SetHomePos(Frogger);
  LivesLeft--;
  Frogger->LadyFrogOnFrogsBack=false;
  Frogger->LadyFrogActive=false;
  if(LivesLeft>0)
    Frogger->PlayerActive=true;
}

void Frog_Class::Draw(uint8_t OffsetX)
{
  uint8_t FrameIdx=Animation[CurrentAnimationIdx].DefaultFrame;
  if(CurrentFrame>-1)  //  current animation, set it up else we'd use default
    FrameIdx=CurrentFrame;
  gfxData=Animation[CurrentAnimationIdx].Frame[FrameIdx].Gfx;
  width=Animation[CurrentAnimationIdx].Frame[FrameIdx].width;
  height=Animation[CurrentAnimationIdx].Frame[FrameIdx].height;
  tft->drawRGBBitmap(x+OffsetX,y,gfxData,width,height);
}



void Frog_Class::MoveLadyFrog(Frogger_Class *Frogger)
{
  // Should I have derived the lady frog from the main frog class, perhaps, but I didn't.

  if(Frogger->LadyFrogLog==0)
    return;
   // If no current animation is happening then just move if on a floating object and check if still on screen
  if(CurrentFrame==-1)
    x+=Velocity_H;                    // This horizontal velocity would be gained on a floating object
    
  if((CurrentAnimationIdx==JUMP_RIGHT)|(CurrentAnimationIdx==JUMP_LEFT))
  {
    if(AnimationCounter==Animation[CurrentAnimationIdx].Frame[CurrentFrame].ClicksToPlayFor)
      x+=Animation[CurrentAnimationIdx].Frame[CurrentFrame].XTotalMoveAmount;
      
    // ensure always hops neatly on log and does not end up over the side a bit
    if(x<Frogger->LadyFrogLog->x)
      x=Frogger->LadyFrogLog->x;
    if(x+width+2>Frogger->LadyFrogLog->x+Frogger->LadyFrogLog->width)    //+2 is a bit of a fudge
      x=(Frogger->LadyFrogLog->x+Frogger->LadyFrogLog->width)-width;
    AnimationCounter--;
    if(AnimationCounter==0)             // This frame has ended move to next frame
    {
      if(CurrentFrame==Animation[CurrentAnimationIdx].NumberOfFrames-1)         // no more frames      
        InitAnimationFrame(CurrentAnimationIdx,-1);                             // animation sequence has completed    
      else
      {
        CurrentFrame++;
        InitAnimationFrame(CurrentAnimationIdx,CurrentFrame);      
      }
    }
  }  
}



void Frog_Class::Move(Frogger_Class *Frogger)
{
  if(LadyFrog)
  {
    MoveLadyFrog(Frogger);
    return;
  }
  // If no current animation is happening then just move if on a floating object and check if still on screen
  if(CurrentFrame==-1)
  {
    x+=Velocity_H;                    // This horizontal velocity would be gained on a floating object
    // has it gone off screen due to being on a floating object, if so lose life      
    if(x<0)                                               // if off screen on left then lose life
    {
      Velocity_H=0;
      SetAnimation(DROWNING_DEATH,Frogger);
    }
      
    if(x+width>=SCREEN_WIDTH)           // if off screen on right then lose life
    {
      Velocity_H=0;
      SetAnimation(DROWNING_DEATH,Frogger);
    }
    return;
  }

  // If we are jumping up or down cancel any x movement
  if((CurrentAnimationIdx==JUMP_UP)|(CurrentAnimationIdx==JUMP_DOWN))
    Velocity_H=0;     
  else  
    x+=Velocity_H;                    // This horizontal velocity would be gained on a floating object
    
  if((AnimationCounter==Animation[CurrentAnimationIdx].Frame[CurrentFrame].ClicksToPlayFor)&(CurrentAnimationIdx!=DROWNING_DEATH)&(CurrentAnimationIdx!=COLLISION_DEATH))              // if true then not yet moved, move object ONLY IF NOT DEATH ANIMATION
  {

    x+=Animation[CurrentAnimationIdx].Frame[CurrentFrame].XTotalMoveAmount;
    y+=Animation[CurrentAnimationIdx].Frame[CurrentFrame].YTotalMoveAmount;
    if(y>FROG_Y_START)                                    // don't allow going lower than should be possible
    {
      y=FROG_Y_START;
      CurrentFrame=-1;                                  // cancel any animation
    }    
    else 
    {
      if(x<0)                                    // don't allow going left more than should be possible
      {
        x=0;    
        CurrentFrame=-1;                                  // cancel any animation
      }
      else
      {
        if(x+width>=SCREEN_WIDTH)                                    // don't allow going right more than should be possible
        {
          x=SCREEN_WIDTH-width;
          CurrentFrame=-1;                                  // cancel any animation
        }
      }
    }
  }
  AnimationCounter--;
  if(AnimationCounter==0)             // This frame has ended move to next frame
  {
    if(CurrentFrame==Animation[CurrentAnimationIdx].NumberOfFrames-1)                                            // no more frames
    {
      // animation sequence has completed
      InitAnimationFrame(CurrentAnimationIdx,-1);
      if((CurrentAnimationIdx==DROWNING_DEATH)|(CurrentAnimationIdx==COLLISION_DEATH))
        DecrementLives(Frogger);
    }
    else
    {
      CurrentFrame++;
      InitAnimationFrame(CurrentAnimationIdx,CurrentFrame);
      
    }
  }
  
}

void Frog_Class::SetHomePos(Frogger_Class *Frogger)
{
  y=FROG_Y_START;
  x=FROG_X_START;  
  FurthestHopPos=0;                                             // Frogger scores 10pts per forward jump but to stop people
  CurrentHopPos=0;  
  TimeStart=millis();                                           // Set to start again
  SetAnimation(JUMP_UP,Frogger);
  InitAnimationFrame(JUMP_UP,-1);
}




// Animation class
////////////////////////////////////////////////////////////////////////////////////////////////////////



AnimationFrame_Class Animation_Class::AddFrame(uint8_t FrameIdx,const unsigned short *Gfx,uint8_t width, uint8_t height,uint8_t XMov,uint8_t YMov,uint8_t ClicksToPlayFor)
{
  // adds a frame of animation to the animation class, if frame already exists then it is replaced
  Frame[FrameIdx].Gfx=Gfx;
  Frame[FrameIdx].width=width;
  Frame[FrameIdx].height=height;
  Frame[FrameIdx].XTotalMoveAmount=XMov;
  Frame[FrameIdx].YTotalMoveAmount=YMov;
  Frame[FrameIdx].ClicksToPlayFor=ClicksToPlayFor;
  NumberOfFrames++;
  return Frame[FrameIdx];
}




// Text Handling, as these are very specific to Frogger (i.e. way font is defined etc. and it would
// be specific to most games it has not been put into the 7735 driver code (which has it's own font stuff)


void DrawText(TFT_eSPI_extended* tft, char* text, uint16_t x, uint16_t y, uint16_t colour)
{
  // draw the text passed in at coords passed in a colour
  // note, nnot attempt to wrap text is made, this a simple routine
  // plotting text onto a old arcade game screen. Do not exceed
  // 255 chars as will just wrap back to plotting start of text
  unsigned char i = 0;
  while (text[i] != 0)
  {
    DrawChar(tft, text[i], x + (i * 4), y, colour); // 4 denotes spacing between chars, chage as required.
    i++;

  }
}

void DrawChar(TFT_eSPI_extended* tft, char theChar, uint16_t x, uint16_t y, uint16_t colour)
{
  // draws single char at pos specified and colour

  char StartIdx;                // start pos in font array for this letter
  char i;                       // index into pixel defintion of this char
  unsigned char Mask;     //  a msk so we can look at single pixels to see if set ot not

  // Is this a number or letter, we need to know to look in the correct place in the font array
  if (theChar < 65) // must be number
  {
    StartIdx = theChar - 48;            // as in ascii number 0 starts at 8
  }
  else
  {
    StartIdx = 10 + theChar - 65;       //letter dfinitions start at 10 and letter "A" at ascii pos 65
    // obviouslt could have simplified this line, but want it
    // readable
  }

  if ((StartIdx < 0) | (StartIdx > 35)) // invalid character, return doing nothing
    return;
  // StartIdx now holds pos ot this character definition, plot out the defintion at this location

  for (i = 0; i < 4; i++)
  {
    // we shift out each pixel
    Mask = B00001000;                         // we are interested in first pixel of row first
    for (int pixel = 0; pixel < 4; pixel++)
    {
      if (pgm_read_byte(&Font[StartIdx][i])&Mask)             //if true pixel is set
      {
        tft->drawPixel(x + pixel, y, colour);      // write it out
      }
      Mask = Mask >> 1;                            // shift pattern so we look at next pixel across
    }
    y++;                                            // move down to next row
  }

}


// Draw a RAM-resident 16-bit image (RGB 5/6/5) with a "Simple" mask
// Values of 0 are transparent, any other is opaque, Based on Adafruit
// based on Adafruit routine of same name
void drawRGBBitmap(TFT_eSPI_extended* tft,int16_t x, int16_t y,const unsigned short *bitmap,int16_t w, int16_t h) 
{
    //startWrite();
    for(int16_t j=0; j<h; j++, y++) 
    {
        for(int16_t i=0; i<w; i++ ) 
        {
          if( bitmap[j * w + i]>0)
                tft->drawPixel(x+i, y, bitmap[j * w + i]);
        }
    }
    //endWrite();
}
