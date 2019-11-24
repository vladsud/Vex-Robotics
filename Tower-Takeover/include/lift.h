#pragma once

#include "pros/adi.hpp"

class Lift
{
private:
  bool m_moving = false;
  PidImpl pid {1 /*precision*/};

public:
  pros::ADIAnalogIn m_anglePot;
  Lift();
  void Update();
  void SetLiftMotor(int speed);
  bool IsMoving() { return m_moving; }
};

Lift& GetLift();
void OpenArmsOnStart();
