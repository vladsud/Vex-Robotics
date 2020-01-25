#pragma once

#include "pros/adi.hpp"

class CubeTray
{  
private:
  const int cubeTrayOut = 2575;
  const int cubeSlowerOut = 1750;
  const int cubeArmsUp = 1450;
  const int restValue = 1350;
  const int cubeInitialization = 1270;
  const int outABitValue = 1400;

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
