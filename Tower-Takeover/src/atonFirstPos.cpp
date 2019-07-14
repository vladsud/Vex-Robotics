#include "aton.h"
#include "atonFirstPos.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

const unsigned int distanceTillWall = 6900;
const int angleToMoveToFlags = 6;
const int angleToShootFlags = 0;
const int distanceToCap = 5350;

void RunAtonFirstPos()
{
    auto &main = GetMain();
    auto timeBegin = main.GetTime();
}
