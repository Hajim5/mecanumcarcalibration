#ifndef Mecanum_Titan_H

#define Mecanum_Titan_H

void moveForwardIntersections(int target);
void turnRight90();
void turnRight90_O();
void turn180_R();
void turn180_RO();
void moveback(int target);
void turnLeft90();
void turnLeft90_O();
void turn180_L();
void turn180_LO();

//Calibrated
void leftmove(int target);
void leftmove_O(int target);

//Uncalibrated
void rightmove();

//--------------------------------------------------
//Unfinished part
//--------------------------------------------------

#endif