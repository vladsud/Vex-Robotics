#pragma once
#include "main.h"

class Cubetray
{
    Direction m_direction = Direction::None;

  private:
    int targetSpeed = 50;
    int currentSpeed = 0;
    int K = .5;

  public:
    void Update();
};