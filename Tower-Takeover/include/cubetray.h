#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Cubetray
{
  
  private:
    const int upValue = 1400;
    const int initValue = 2000;
    const int midValue = 2250;
    const int restValue = 2850;

  public:
    Cubetray();
    void Update();
    
    pros::ADIAnalogIn m_anglePot;
};