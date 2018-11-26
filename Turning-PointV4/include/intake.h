#pragma once
#include "main.h"

class Intake
{
    enum class Direction
    {
        None,
        Up,
        Down,
    };

    Direction m_direction = Direction::None;
    bool m_doublePressed = false;

  public:
    void Update();
    void SetIntakeMotor(int speed);
};


class Descorer
{
    int m_count = 0;
  public:
    void Update();
};