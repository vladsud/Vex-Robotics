#pragma once
#include "main.h"

class Lift
{
  private:
    int targetSpeed = 90;
    int currentSpeed = 0;
    int K = .5;

  public:
    void Update();
    void SetLiftMotor(int speed);
};