#pragma once
#include "main.h"

class Spinner
{
  enum class SpinnerState
  {
    Off = 0,
    On = 1,
    Automatic = 2,
  };

  bool m_oldSpinnerState = false;
  SpinnerState m_state = SpinnerState::Off;
  int m_target = 0;
  int m_lastRead = 0;
  int m_lastError = 0;
  int m_power = 0;
  int m_cyclesAfterStopped = 0;

public:
  //Spinner Control
  bool GetSpinner();
  static bool GetAutoSpinner();
  void SetSpinnerMotor(float speed);
  void ReadInputs();
  void Start();
  void Stop();
  // Sequence Lift up, spin, lift down
  void RotateAndDown();
  void DebugRun();
  void Update();
};

extern Spinner spinner;