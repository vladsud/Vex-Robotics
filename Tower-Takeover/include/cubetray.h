#pragma once

class CubeTray
{  
public:
  static const int cubeTrayOut = 1440;
  static const int cubeSlowerOut = 2300;
  static const int cubeArmsUp = 2500;
  static const int restValue = 2885;
  static const int outABitValue = 2700;

private:  bool m_moving = false;
  PidImpl pid {50};
  int m_power = 0;
  int m_tick = 0;
  bool rumbled = false;

  pros::ADIAnalogIn m_anglePot;

public:
    const int cubeArmsCanUp = 2900;
    CubeTray();
    void Update();
    bool IsMoving() { return m_moving; }
    int get_value();

    bool isForced = false;

};

CubeTray& GetCubeTray();
void OpenTrayOnStart();
