#include <Vector.h> // https://github.com/janelia-arduino/Vector
#include "GraphicsData.h"
#include "Graphics.h"
#include "Frogger.h"
#include "Levels.h"

void InitLevel(Frogger_Class *Frogger,uint8_t LevelNumber)
{
  ClearAllObjects(Frogger);
  switch (LevelNumber)
  {
    case 1:InitLevel1(Frogger);break;
  }
    
}

void ClearAllObjects(Frogger_Class *Frogger)
{
  MovingObject_Class *ThisObj,*NextObj;

  ThisObj=Frogger->GameObjectListStart;
  while(ThisObj!=0)
  {
    NextObj=ThisObj->NextObj;
    delete(ThisObj);
    ThisObj=NextObj;
  }
  
  //Clear lady frog log pointers from lady frog log lane
  for(int i=0;i<5;i++)
    Frogger->LadyFrogLogs[i]=0;
  Frogger->LadyFrogIdx=0;
  Frogger->LadyFrogLog=0;
}


void InitLevel1(Frogger_Class *Frogger)
{  
  static int16_t storage1[3] = {40,74,112};
  Vector<int16_t> lane1(storage1, 3);
  VehicleLane(Frogger,CarLeft,-0.2,lane1,105,CarLeftWidth,CarLeftHeight);        //22s

  static int16_t storage2[3]= {48,80,-15};
  Vector<int16_t> lane2(storage2,3);
  VehicleLane(Frogger,BullDozer,0.24,lane2,97,BullDozerWidth,BullDozerHeight);   //18s

  static int16_t storage3[3] = {48,80,114};
  Vector<int16_t> lane3(storage3,3);
  VehicleLane(Frogger,PurpleCar,-0.3,lane3,90,PurpleCarWidth,PurpleCarHeight);   //14s

  static int16_t storage4[1] = {-16};
  Vector<int16_t> lane4(storage4,1);
  VehicleLane(Frogger,CarRight,0.4,lane4,80,CarRightWidth,CarRightHeight);       //10s

  static int16_t storage5[2] = {60,104};
  Vector<int16_t> lane5(storage5,2);
  VehicleLane(Frogger,Lorry,-0.6,lane5,73,LorryWidth,LorryHeight);               //7s

  static int16_t storage6[8] = {0,1, 33,0, 65,0, 97,0};
  Vector<int16_t> lane6(storage6,8);
  TurtleLane(Frogger,-0.6,lane6,56,3);                                           //7s

  static int16_t storage7[4] = {-29,18,58,98};
  Vector<int16_t> lane7(storage7,4);
  LogLane(Frogger,0.24,lane7,LADY_FROG_LANE_Y,1,40);                             //18s
  
  static int16_t storage8[3] = {-52,12,76};
  Vector<int16_t> lane8(storage8,3);
  LogLane(Frogger,1.2,lane8,41,4,32);                                            //3.5s
  
  static int16_t storage9[8] = {19,1,  47,0,  75,0, 102,0};
  Vector<int16_t> lane9(storage9,8);
  TurtleLane(Frogger,-0.6,lane9,32,2);                                           //7s     
  
  static int16_t storage10[4] = {-39,2,46,90};
  Vector<int16_t> lane10(storage10,4);
  LogLane(Frogger,0.37,lane10,25,2,32);                                           //11s  
}

void VehicleLane(Frogger_Class *Frogger,const unsigned short *gfx,float Velocity,  Vector<int16_t> & XData,uint8_t y,uint8_t width,uint8_t height)
{
  for(uint8_t i=0;i<XData.size();i++)
    AddObject(new MovingObject_Class(&Frogger->tft,gfx,width,height,XData[i],y,Velocity),Frogger);    
}

void LogLane(Frogger_Class *Frogger,float Velocity, Vector<int16_t> & XData,uint8_t y,uint8_t Length,uint8_t DistToTravOffScr)
{
  Log_Class *Log;
  for(uint8_t i=0;i<XData.size();i++)    {
    Log=new Log_Class(&Frogger->tft,XData[i],y,Velocity,Length,DistToTravOffScr);
    AddObject(Log,Frogger);
    // If this is a log that can have a lady frog on then add to the special list
    if(y==LADY_FROG_LANE_Y)
    {
      Frogger->LadyFrogLogs[Frogger->LadyFrogIdx]=Log;  
      Frogger->LadyFrogIdx++;
    }
  }
}

void TurtleLane(Frogger_Class *Frogger,float Velocity, Vector<int16_t> & XData,uint8_t y,uint8_t NumTurtlesPerGroup)
{
  // XData format :XData+0 -> x ord start for this group,  XData+1 ->diving or not -0 not 1 diving
  for(uint8_t i=0;i<XData.size();i+=2)    
  {
    AddObject(new Turtle_Class(&Frogger->tft,XData[i],y,Velocity,XData[i+1],NumTurtlesPerGroup),Frogger);      
  }
}


void Lane1(Frogger_Class *Frogger,float Velocity, Vector<int16_t> & XData)
{
  for(uint8_t i=0;i<XData.size();i++)
    AddObject(new MovingObject_Class(&Frogger->tft,CarLeft,CarLeftWidth,CarLeftHeight,XData[i],105,Velocity),Frogger);  // 0.4 about 24 secs to travel across
    
}

void AddObject(MovingObject_Class *ThisObj,Frogger_Class *Frogger)
{
  // adds a moving object to the list of game objects
  
  if(Frogger->GameObjectListStart==0)              // empty at present, first entry
  {
    Frogger->GameObjectListStart=ThisObj;          // starts and ends with this entry
    Frogger->GameObjectListEnd=ThisObj;
  }
  else{
    Frogger->GameObjectListEnd->NextObj=ThisObj;
    ThisObj->PreviousObj=Frogger->GameObjectListEnd;
    Frogger->GameObjectListEnd=ThisObj;
  }
}
