#pragma once
#include "main.h"

class Intake
{
    bool intake = false;
    bool oldIntakeState = false;

  public:
    bool GetIntake();
    void SetIntakeMotor(float speed);
    void Update();
};