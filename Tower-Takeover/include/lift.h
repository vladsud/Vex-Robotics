#pragma once
#include "main.h"
#include "pros/adi.hpp"

class Lift
{
private:
  int count = 0;
  bool m_moving = false;

public:
  pros::ADIAnalogIn m_anglePot;
  Lift();
  void Update();
  void SetLiftMotor(int speed);
  bool IsMoving() { return m_moving; }
};