#pragma once
#include "main.h"

class Cubtray
{
    enum class Direction
    {
        None,
        Up,
        Down,
    };
    Direction m_direction = Direction::None;

  public:
    void Update();
};