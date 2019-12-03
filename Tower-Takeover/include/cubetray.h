#pragma once

#include "pros/adi.hpp"

class CubeTray
{  
private:
  const int upValue = 2385;
  const int midValue = 1432;
  const int restValue = 820;

  bool m_moving = false;
  PidImpl pid {100};

public:
    CubeTray();
    void Update();
    bool IsMoving() { return m_moving; }
    
    pros::ADIAnalogIn m_anglePot;
};

CubeTray& GetCubeTray();
