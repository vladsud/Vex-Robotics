#pragma once
#include "main.h"

class Drive
{
  int m_forward = 0;
  float m_turn = 0;
  int m_overrideForward = 0;
  float m_overrideTurn = 0;

  float m_ErrorIntergral = 0;

  bool KeepDrection(int forward, float turn)
  {
    return
        (m_turn == 0 && turn == 0) ||
        (m_forward == 0 && forward == 0) ||
        (m_turn == turn && m_forward == forward);
  }
public:
  int m_distance = 0;
  int m_distanceTurn = 0;
  int m_gyro = 0;

public:
  static int GetMovementJoystick(unsigned char joystick, unsigned char axis, int minValue);

  int GetForwardAxis();
  float GetTurnAxis();

  Drive() { ResetEncoders(); }
  void SetLeftDrive(int speed);
  void SetRightDrive(int speed);
  void DebugDrive();
  void AutoDriveForward(float distance, int power);
  void OverrideInputs(int forward, float turn);
  void ResetEncoders();
  void Update();
};
