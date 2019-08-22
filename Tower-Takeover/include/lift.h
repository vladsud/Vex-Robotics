#pragma once
#include "main.h"

class Lift
{
  public:
    void Update();
    void SetLiftMotor(int speed);
    bool IsInitialized();
    void Initialize();

private:
    bool m_initialize = false;
    int m_initializationDistance = 100;
};