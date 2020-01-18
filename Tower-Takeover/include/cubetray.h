#pragma once

#include "pros/adi.hpp"

class CubeTray
{  
private:
  const int cubeTrayOut = 2600;
  const int cubeSlowerOut = 1750;
  const int cubeArmsUp = 1830;
  const int restValue = 1425;
  const int cubeInitialization = 1324;

  bool m_moving = false;
  PidImpl pid {50};
  int m_power = 0;

public:
    CubeTray();
    void Update();
    bool IsMoving() { return m_moving; }
    
    pros::ADIAnalogIn m_anglePot;
};

CubeTray& GetCubeTray();
void OpenTrayOnStart();
