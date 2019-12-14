#pragma once

#include "pros/adi.hpp"

class Lift
{
private:
  bool m_moving = false;
  PidImpl pid {1 /*precision*/};

  float currentPosition;
  int m_count = 0;

public:
  Lift();
  void Update();
  void SetLiftMotor(int speed);
  bool IsMoving() {return m_moving; }

  float get_value();

  int ArmsInitialization = 1830;
  int ArmsMidPos = 1980;
  int ArmsLowPos = 1550;
  int ArmsTrayCanMoveDown = 1200;
  int RestPos = 0;

};

Lift& GetLift();
void OpenArmsOnStart();
void TowerMid();
