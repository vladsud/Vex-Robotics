#pragma once

#include "pros/adi.hpp"

class Lift
{
private:
  bool m_moving = false;
  PidImpl pid {50 /*precision*/};

  int currentPosition;
  int m_count = 0;

public:
  Lift();
  void Update();
  void SetLiftMotor(int speed);
  bool IsMoving() {return m_moving; }

  int get_value();

  int ArmsInitialization = 1830;
  int ArmsMidPos = 1950;
  int ArmsLowPos = 1550;
  int ArmsTrayCanMoveDown = 1200;
  int RestPos = 0;

  int DoubleCube = 600;

};

Lift& GetLift();
void ArmsMid();
void TowerLow();
void TowerRest();