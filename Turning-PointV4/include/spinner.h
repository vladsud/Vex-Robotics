#pragma once
#include "main.h"

class Spinner
{
    bool m_oldSpinnerState = false;
    bool m_on = false;
    int m_target = 0;
    int m_lastRead = 0;
    int m_lastError = 0;
    int m_power = 0;
    int m_cyclesAfterStopped = 0;

  public:
    //Spinner Control
    bool GetSpinner();

    void SetSpinnerMotor(float speed);
   
    void ReadInputs();

    void Start();

    void Stop();

    void DebugRun();

    void Update();
};