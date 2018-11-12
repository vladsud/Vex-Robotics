#pragma once
#include "main.h"

class Intake
{
    bool m_intake = false;
    bool m_oldIntakeState = false;

  public:
    bool GetIntake();
    void SetIntakeMotor(float speed);
    void Update();
};