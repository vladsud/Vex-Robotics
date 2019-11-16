#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Cubetray
{
  
  private:
    Direction m_direction = Direction::None;
    int totalError = 0;
    int kI = 0;
    int kP = 0;
    int upValue = 1350;
    int midValue = 2250;

  public:
    Cubetray();
    void Update();
    
    pros::ADIAnalogIn m_anglePot;
};