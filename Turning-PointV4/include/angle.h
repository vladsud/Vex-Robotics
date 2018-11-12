#pragma once
#include "main.h"

class Angle
{
    bool angleUp = false;
    bool angleDown = false;

  public:
    bool GetAngleUp();
    bool GetAngleDown();
    void SetAngleMotor(float speed);
    void Update();
};