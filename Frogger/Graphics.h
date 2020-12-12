#include "WIO_TFT.h"

class Frogger_Class;
class MovingObject_Class;

class BaseObject_Class
{
    //base class for all graphical objects
  public:
    float x;
    int16_t y;
    int8_t width, height;
    const unsigned short* gfxData;
    bool Mask = false;
    TFT_eSPI_extended* tft;
    String ClassName = "";          // for debugging purposes only


    //constructors
    explicit BaseObject_Class(TFT_eSPI_extended *tft, const unsigned short* gfx, int16_t width, int16_t height, float x, int16_t y);

    void Init(TFT_eSPI_extended *tft, const unsigned short* gfx, int16_t width, int16_t height, float x, int16_t y, uint8_t DistOffScreen);

    virtual bool Collision(MovingObject_Class *Object, Frogger_Class *Frogger);
    bool CollidedWith(MovingObject_Class *Object);

    // drawing routines
    virtual void Draw(uint8_t OffsetX);

};

class AnimationFrame_Class
{
  public:
  const unsigned short *Gfx=0;
  int8_t XTotalMoveAmount=0;         // total amount to move for this frame in x plane
  int8_t YTotalMoveAmount=0;         // total amount to move for this frame in y plane
  uint8_t width;
  uint8_t height;
  uint8_t ClicksToPlayFor=0;           // How many game clicks this animation plays for, bigger = longer
};


// only used by the frog itself! but implemented "properly" for use in future games
class Animation_Class
{
  public:
  uint8_t NumberOfFrames=0;
  AnimationFrame_Class Frame[4];        // ONLY 3 FRAMES USED FOR JUMPING, 4 FOR DEATHS, WASTING SPACE HERE!
  uint8_t DefaultFrame=2;
  AnimationFrame_Class AddFrame(uint8_t FrameIdx,const unsigned short *Gfx,uint8_t width, uint8_t height,uint8_t XMov,uint8_t YMov,uint8_t ClicksToPlayFor);
};


//Any object that can move will derive (be based on) this class, cars use it directly
class MovingObject_Class: public BaseObject_Class
{
  public:
    MovingObject_Class *PreviousObj=0;                      // pointer to previous game object in the list that this is in
    MovingObject_Class *NextObj=0;                          // pointer to next game object in the list that this is in
    float Velocity_H;                                     // Horizontal velocity minus going left, positive right and magnitude is speed
    // Note that only one object in this game can go in a vertical direction
    // and that's the players frog, that will have it's own derived class from this.

    // floating object. This enables us to make this object travel with the floating
    // object. A value of -100 means that it is not currently on a floating object
    bool JumpingInXOrd=false;                                // if set then frog currently jumping in x ord direction (lect or right)
    
    Animation_Class Animation[6];                       // stores any animations
    uint8_t CurrentAnimationIdx=0;                                    // which animation to play, 
    int8_t CurrentFrame=-1;                               //  -1 means display default and no animation
    bool OnFloatingObject;                                // true if the last round of collision detection showed frog on floating object
    
    uint8_t AnimationCounter;

    uint8_t DistanceToTravelOffScreen;                    // annolyingly need this, I think in original it may have had some hardware wrap-around

    using BaseObject_Class::BaseObject_Class;
    MovingObject_Class(TFT_eSPI_extended *tft, const unsigned short* gfx, int16_t width, int16_t height, float x, int16_t y, float Velocity);
    MovingObject_Class(TFT_eSPI_extended *tft, const unsigned short* gfx, int16_t width, int16_t height, float x, int16_t y, float Velocity, uint8_t DistOffScreen);

    void Init(TFT_eSPI_extended *tft, const unsigned short* gfx, int16_t width, int16_t height, float x, int16_t y, float Velocity, uint8_t DistOffScreen);

    virtual void DecrementLives(Frogger_Class *Frogger);  // override in any subclass, not implemented
    void SetAnimation(uint8_t Index,Frogger_Class *Frogger);
    void InitAnimationFrame(uint8_t AnimationIdx,uint8_t AnimationFrame);
};

class FloatingObject_Class: public MovingObject_Class
{ public:

    FloatingObject_Class(TFT_eSPI_extended *tft, int16_t width, int16_t height, float x, int16_t y, float Velocity, uint8_t DistOffScreen);
    bool Collision(MovingObject_Class *Object, Frogger_Class *Frogger) override;
};

class Log_Class: public FloatingObject_Class
{
  public:
    uint8_t Length;                 // number of middle sections, logs consist of start, middle(s) and end, 1,2, o3 for frogger

    Log_Class(TFT_eSPI_extended *tft, float x, int16_t y, float Velocity, uint8_t Length);
    Log_Class(TFT_eSPI_extended *tft, float x, int16_t y, float Velocity, uint8_t Length, uint8_t DistOffScreen);
    // drawing routines
    void Draw(uint8_t OffsetX) override ;

};



class Turtle_Class: public FloatingObject_Class
{

  public:
    uint8_t NumTurtlesPerGroup;     // number of turtles in this group, game has usually 2 or 3
    bool    Diving;                 // Are these diving turtles, true if yes else false
    uint8_t Frame;                  // Which frame of animation to display next,
    uint8_t AnimationSpeed;         // Speed that animation plays at, higher=slower

    Turtle_Class(TFT_eSPI_extended *tft, float x, int16_t y, float Velocity, bool Diving, uint8_t NumTurtlesPerGroup);
    // drawing routines
    void Draw(uint8_t OffsetX) override ;
};


class Frog_Class: public MovingObject_Class
{
  public:
    uint8_t LivesLeft;
    uint8_t FurthestHopPos=0;                                             // Frogger scores 10pts per forward jump but to stop people
    uint8_t CurrentHopPos=0;                                              // going forwards/backwards/forwards to points press
                                                                        // and get more pts the game tracks the furthest hop pos
                                                                        // and only awards pts when they go past last highest pos
    bool LadyFrog=false;                                                // if true this is the lday frog not the player frog (differen graphics)
                                                                        
    uint32_t TimeStart;                                                   // Start time in millis from when frog starts
    Frog_Class(TFT_eSPI_extended *tft, float x, int16_t y,bool LadyFrog);
    void DecrementLives(Frogger_Class *Frogger);
    void SetHomePos(Frogger_Class *Frogger);
    void Draw(uint8_t OffsetX) override ;
    void Move(Frogger_Class *Frogger);
    void MoveLadyFrog(Frogger_Class *Frogger);
};


// Text Handling
void DrawText(TFT_eSPI_extended* tft, char* text, uint16_t x, uint16_t y, uint16_t colour);
void DrawChar(TFT_eSPI_extended* tft, char theChar, uint16_t x, uint16_t y, uint16_t colour);
void drawRGBBitmap(TFT_eSPI_extended* tft, int16_t x, int16_t y, const unsigned short *bitmap, int16_t w, int16_t h);
