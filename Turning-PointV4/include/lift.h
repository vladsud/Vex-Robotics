#pragma once
#include "main.h"

class Lift
{
    enum class LiftState
    {
      Off,
      Up,
      Down,
    };

    int m_LiftSpeed = 0;
    LiftState m_state = LiftState::Off;
    bool m_automaticKey = false;

  public:
    int CurrentHeight();
    bool GetLiftUp();
    bool GetLiftDown();
    void SetLiftMotor(float speed);
    void Update();

    // Spinner calls it when in automatic mode rotating cones
    void MoveDown();
};

extern Lift lift;