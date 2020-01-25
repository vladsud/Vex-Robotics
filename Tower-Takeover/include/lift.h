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
  int ArmsMidPos = 2020;
  int ArmsLowPos = 1625;
  int ArmsTrayCanMoveDown = 1200;
  int RestPos = 0;

};

Lift& GetLift();
void TowerMid();
