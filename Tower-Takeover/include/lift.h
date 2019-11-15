#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Lift
{
  private:

    int totalTrayError = 0;
    int totalArmError = 0;

  public:
    pros::ADIAnalogIn m_anglePot;
    Lift();
    void Update();
    void SetLiftMotor(int speed);
};