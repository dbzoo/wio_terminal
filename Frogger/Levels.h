#include <Vector.h> // https://github.com/janelia-arduino/Vector


void InitLevel(Frogger_Class *Frogger,uint8_t LevelNumber);
void InitLevel1(Frogger_Class *Frogger);
void ClearAllObjects(Frogger_Class *Frogger);
void AddObject(MovingObject_Class *ThisObj,Frogger_Class *Frogger);
void VehicleLane(Frogger_Class *Frogger,const unsigned short *gfx,float Velocity,  Vector<int16_t> & XData,uint8_t y,uint8_t width,uint8_t height);
void LogLane(Frogger_Class *Frogger,float Velocity, Vector<int16_t> & XData,uint8_t y,uint8_t Length,uint8_t DistToTravOffScr);
void TurtleLane(Frogger_Class *Frogger,float Velocity, Vector<int16_t> & XData,uint8_t y,uint8_t NumTurtlesPerGroup);
