// The frogger class contains main game global elements
//#ifndef FROGGER_H
//#define FROGGER_H
#include "WIO_TFT.h"

// Useful defines       

// SCREEN STUFF
// note these values below are after convesion to fit on this screen, original game screen was 224x256  
#define RIVER_END 63                        // where the blue of the river ends
#define BOTTOM_PAVEMENT_Y 112               //
#define SCREEN_WIDTH 112                    // width of actual game screen not Frogger.tft   
#define TFT_WIDTH 128        
#define TFT_HEIGHT 128        
#define X_START 8                           // actual x start of graphics on display
#define SCREEN_END_X X_START+SCREEN_WIDTH   // END OF ACTUAL HORIZONTAL GAME SCREEN NOT tft
#define DISTANCE_TO_TRAVEL_OFF_SCREEN 8      // AFFECTS TIMING OF WHEN OBJECTS REAPPEAR AT OTHER SIDE,higher=longer
#define HEDGE_PIECE_WIDTH 4
#define HEDGE_PIECE_HEIGHT 4
#define NUMBER_VERTICAL_HEDGE_PIECES 3      // how many hedge tile pieces are present vertically
#define HEDGE_Y_START 12
#define SPACE_BETWEEN_TURTLES 0             // from tail to head
#define TIMEPERFROG 32                      // Time allowed in seconds to get one frog home
#define TURTLE_ANIMATION_SPEED 3            // Speed that animation plays at, higher=slower
#define TURTLE_DIVING_ANIMATION_SPEED 7     // Speed that animation plays at, higher=slower

// flies, lady frogs and frogs
#define NOT_FLOATING_ON_ANYTHING -100       // if the relative position to a floating object is this value then deemed not to be on a floating object
#define ODDS_OF_FLY 170                     // odds of fly appearing, higher the number the less likely, this seems to give a randomness
                                            // close to the arcade version of between 3 and 12 secs between flys
#define TIME_FLY_ACTIVE 2000                // in ms, how long fly stays on screen for

#define ODDS_OF_LADY_FROG 300               // The higher the number the less likely, this seems to give a similar chance as the arcade game
#define LADY_FROG_LANE_Y 49                 // Y pos of the logs where a lady frog can appear
#define DELAY_BETWEEN_LADY_FROG_MOVES 1000  // The time in millis between lady frog movements


//PLAYERS FROG 
#define BUT_LEFT WIO_5S_LEFT
#define BUT_RIGHT WIO_5S_RIGHT
#define BUT_UP WIO_5S_UP
#define BUT_DOWN WIO_5S_DOWN
#define BUT_PLAYER_1 WIO_KEY_C              // Changed in V6 as 25 was DAC output (used for audio)                     
#define BUT_PLAYER_2 WIO_KEY_A              // 2 Player not implemented in code
#define FROG_Y_START 113
#define FROG_X_START 60
#define START_LIVES 3                       // original arcade could choose between 3,5,7 and apparantly 256, although only seen this reference once for 256

#define JUMP_UP 0                           // These are animation codes for which animation to show
#define JUMP_RIGHT 1
#define JUMP_DOWN 2
#define JUMP_LEFT 3
#define DROWNING_DEATH 4
#define COLLISION_DEATH 5

#define JUMP_SPEED 2                        // SPEED OF FROG JUMP, bigger = slower
#define ALL_HOME_ANIMATION_INTERVAL 750     //  Time in ms between each frog dissapearing when they are all home


class Frogger_Class
{
  public:
  TFT_eSPI_extended &tft;
  bool GameInPlay=false;                                                // is the game in play or not?
  uint8_t Level=1;                                                      // Level, wraps around back to 1 after 255, not sure if arcade did this
  bool PlayerActive=false;                                              // is the player allowed to move?
  uint32_t Score=0;
  uint32_t HiScore=5102;
  uint32_t TimeElapsed;                                                 // how much time as elapsed from start of this frog in millis                                            
  int8_t TimeLeft;                                                      // Time Left in game beats
  bool UpPressed=false;
  bool DownPressed=false;
  bool LeftPressed=false;
  bool RightPressed=false;
  // These are the x ords of the "homes" in the hedge, used to help detect if the frog is "home"
  uint16_t Homes[5]={HEDGE_PIECE_WIDTH,HEDGE_PIECE_WIDTH*7,HEDGE_PIECE_WIDTH*13,HEDGE_PIECE_WIDTH*19,HEDGE_PIECE_WIDTH*25};
  bool FrogHome[5]={0,0,0,0,0};                                         // if true a frog is in that home
  bool AllHome=false;                                                   // true if all frogs home (starts end of level seqeunce if true)
  uint16_t AllHomeTimer;                                                // Set to current millis() when all frogs home to keep track of end of
                                                                        // level animation
  uint8_t FlyIdx=5;                                                     // which "home" is fly in, >4 = none (0 to 4 valid)
  uint32_t FlyTime;                                                     // Helps keeps track of how long fly has been on screen
  
  
  // All active (i.e. not scenary, moving) obects except Frogger himself
  MovingObject_Class *GameObjectListStart=0;                                      // The start of the linked list of game objects
  MovingObject_Class *GameObjectListEnd=0;                                        // The end of the linked list of game objects

  Frog_Class *Frog;                                                     // Frogger itself
  Frog_Class *LadyFrog;                                                 // The Lady Frog
  bool LadyFrogActive=false;                                            // currently in play
  bool LadyFrogCaptured=false;                                          // If true then she is hitching a lift home on frogger
  Log_Class *LadyFrogLogs[5];                                           // array of the possible logs a lady frog can appear on (see LADY_FROG_LANE_Y)
                                                                        // Only first row of logs can have a lady frog on and no more than 5 logs
  uint8_t LadyFrogIdx;                                                  // Next available position in the LadyFrogLogs array for a lady frog log
  Log_Class *LadyFrogLog;                                               // The log the lady frog is on
  uint32_t LadyFrogMoveTime;                                            // start time of last move, used with DELAY_BETWEEN_LADY_FROG_MOVES to set
                                                                        // time between lady from movements
  bool LadyFrogOnFrogsBack;                                             // Indicates currently riding main player frog
                                                       

  Frogger_Class(TFT_eSPI_extended &spr) : tft(spr) {};
  void StartNewGame();
  void GotHome(uint8_t HomeIndex);
  void Physics();
  void NextLevel();
  void ManageLadyFrog();
  bool InRiver();
};
//#endif
