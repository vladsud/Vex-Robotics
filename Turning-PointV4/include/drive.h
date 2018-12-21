#pragma once
#include "main.h"

class Drive
{
  int m_forward = 0;
  float m_turn = 0;
  int m_overrideForward = 0;
  float m_overrideTurn = 0;
  int m_encoderBaseLeft = 0;
  int m_encoderBaseRight = 0;

  float m_ErrorIntergral = 0;

  bool KeepDrection(int forward, float turn)
  {
    // Do not reset m_forward & m_turn on full stop!
    // that screws up autonomous turning.
    return
        (m_turn == 0 && turn == 0 && forward * m_forward >= 0) ||
        (m_forward == 0 && forward == 0) ||
        (m_turn == turn && m_forward == forward);
  }
public:
  int m_distance = 0;
  int m_gyro = 0;

public:
  static int GetMovementJoystick(unsigned char joystick, unsigned char axis, int minValue);

  int GetForwardAxis();
  float GetTurnAxis();

  Drive() { ResetEncoders(); }
  void SetLeftDrive(int speed);
  void SetRightDrive(int speed);
  void DebugDrive();

  // Forward: Positive turn - turn right (clockwise)
  // Backwards: Positive turn - turn left (clockwise)
  void OverrideInputs(int forward, float turn);
  void ResetEncoders();
  void Update();
};
