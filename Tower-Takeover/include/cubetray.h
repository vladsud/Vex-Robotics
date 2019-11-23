#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Cubetray
{
  
private:
  const int upValue = 1350;
  const int initValue = 2000;
  const int midValue = 2250;
  const int restValue = 2850;

  bool m_moving = false;

public:
    Cubetray();
    void Update();
    bool IsMoving() { return m_moving; }
    
    pros::ADIAnalogIn m_anglePot;
};