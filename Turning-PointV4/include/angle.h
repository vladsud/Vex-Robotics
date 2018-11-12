#pragma once
#include "main.h"

class Angle
{
    bool m_angleUp = false;
    bool m_angleDown = false;

  public:
    bool GetAngleUp();
    bool GetAngleDown();
    void SetAngleMotor(float speed);
    void Update();
};