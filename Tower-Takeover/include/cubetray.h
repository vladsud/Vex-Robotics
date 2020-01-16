#pragma once

#include "pros/adi.hpp"

class CubeTray
{  
private:
  const int cubeTrayOut = 2800;
  const int cubeArmsUp = 1830;
  const int cubeSlowerOut = (cubeArmsUp + restValue)/2;
  const int cubeInitialization = 1324;
  const int restValue = 1425;

  bool m_moving = false;
  PidImpl pid {100};

public:
    CubeTray();
    void Update();
    bool IsMoving() { return m_moving; }
    
    pros::ADIAnalogIn m_anglePot;
};

CubeTray& GetCubeTray();
