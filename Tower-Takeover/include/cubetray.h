#pragma once

#include "pros/adi.hpp"

class CubeTray
{  
private:
  const int cubeTrayOut = 3200;
  const int cubeSlowerOut = 1700;
  const int cubeArmsUp = 700;
  const int restValue = 400;
  const int cubeInitialization = 0;
  const int outABitValue = 450;

  bool m_moving = false;
  PidImpl pid {50};
  int m_power = 0;

  float currentRotation;

public:
    CubeTray();
    void Update();
    bool IsMoving() { return m_moving; }
};

CubeTray& GetCubeTray();
void OpenTrayOnStart();
