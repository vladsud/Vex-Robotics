#pragma once

#include "pros/adi.hpp"

class CubeTray
{  
private:
  const int upValue = 2200;
  const int midValue = 1500;
  const int restValue = 806;

  bool m_moving = false;
  PidImpl pid {100};

public:
    CubeTray();
    void Update();
    bool IsMoving() { return m_moving; }
    
    pros::ADIAnalogIn m_anglePot;
};

CubeTray& GetCubeTray();
