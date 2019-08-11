#pragma once
#include "main.h"

class Intake
{
  public:
    void Update();
    void UpdateIntake(Direction direction);

  private:
    void SetIntakeMotor(int speed);

  private:
    Direction m_direction = Direction::None;
    bool m_doublePressed = false;
    int m_power = 0;
};