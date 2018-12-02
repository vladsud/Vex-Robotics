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
    enum class Direction
    {
        None,
        Up,
        Down,
    };
    int m_count = 0;
    Direction m_direction = Direction::None;
  public:
    void Update();
};