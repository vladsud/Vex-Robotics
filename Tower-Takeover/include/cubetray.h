#pragma once

class CubeTray
{  
private:
  const int cubeTrayOut = 1440;
  const int cubeSlowerOut = 2300;
  const int cubeArmsUp = 2500;
  const int restValue = 2930;
  const int outABitValue = 2800;

  bool m_moving = false;
  PidImpl pid {50};
  int m_power = 0;
  int m_tick = 0;
  bool rumbled = false;

  pros::ADIAnalogIn m_anglePot;

public:
    CubeTray();
    void Update();
    bool IsMoving() { return m_moving; }

    bool isForced = false;

};

CubeTray& GetCubeTray();
void OpenTrayOnStart(int time);
