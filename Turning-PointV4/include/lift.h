#pragma once
#include "main.h"

class Lift
{
    bool m_liftUp = false;
    bool m_liftDown = false;

  public:
    bool GetLiftUp();
    bool GetLiftDown();
    void SetLiftMotor(float speed);
    void Update();
};