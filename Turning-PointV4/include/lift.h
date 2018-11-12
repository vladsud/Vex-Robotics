#pragma once
#include "main.h"

class Lift
{
    bool liftUp = false;
    bool liftDown = false;

  public:
    bool GetLiftUp();
    bool GetLiftDown();
    void SetLiftMotor(float speed);
    void Update();
};