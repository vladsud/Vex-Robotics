#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Cubetray
{
  
    Direction m_direction = Direction::None;
    int totalError = 0;

  public:
    Cubetray();
    void Update();
    
    pros::ADIAnalogIn m_anglePot;
};