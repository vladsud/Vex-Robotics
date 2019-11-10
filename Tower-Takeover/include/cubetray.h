#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Cubetray
{
  
    Direction m_direction = Direction::None;
    
    int totalError = 0;

    pros::ADIAnalogIn m_anglePot;

  public:
    Cubetray();
    void Update();
};