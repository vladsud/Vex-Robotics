#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Lift
{
  private:

    int count = 0;

  public:
    pros::ADIAnalogIn m_anglePot;
    Lift();
    void Update();
    void SetLiftMotor(int speed);
};