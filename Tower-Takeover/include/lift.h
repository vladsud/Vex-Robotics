#pragma once

#include "pros/adi.hpp"

class Lift
{
private:
  bool m_moving = false;
  PidImpl pid {1 /*precision*/};

  float currentPosition;

public:
  Lift();
  void Update();
  void SetLiftMotor(int speed);
  bool IsMoving() {return m_moving; }

  float get_value();

  int ArmsMidPos = 1900;
  int ArmsLowPos = 1420;
  int RestPos = 0;

};

Lift& GetLift();
void OpenArmsOnStart();
