#pragma once
#include "main.h"

class Drive
{
  int m_forward = 0;
  int m_turn = 0;
  int m_overrideForward = 0;
  int m_overrideTurn = 0;

  float m_ErrorIntergral = 0;

public:
  int m_distance = 0;

public:
  static int GetMovementJoystick(unsigned char joystick, unsigned char axis, int minValue);

  int GetForwardAxis();
  int GetTurnAxis();

  void SetLeftDrive(int speed);
  void SetRightDrive(int speed);
  void DebugDrive();
  void AutoDriveForward(float distance, int power);
  void OverrideInputs(int forward, int turn);
  void ResetEncoders();

  void Update();
};
