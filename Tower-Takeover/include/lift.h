#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Lift
{
  private:
    pros::ADIAnalogIn m_anglePot;

  public:
    Lift();
    void Update();
    void SetLiftMotor(int speed);
};