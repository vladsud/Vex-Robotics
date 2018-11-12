#pragma once
#include "main.h"
class Shooter
{
    bool m_shooter = false;

  public:
    bool GetShooter();
    void SetShooterMotor(float speed);
    void Update();
};