#pragma once
#include "main.h"

class Intake
{
  public:
    void Update();
    void UpdateIntake(Direction direction);
    void SetIntakeMotor(int speed);

  private:
    Direction m_direction = Direction::None;
    int m_power = 0;
};