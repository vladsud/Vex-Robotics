#pragma once
#include "main.h"
class Shooter
{
    bool shooter = false;

  public:
    bool GetShooter();
    void SetShooterMotor(float speed);
    void Update();
};