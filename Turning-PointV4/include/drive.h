#pragma once
#include "main.h"

class Drive
{
  int m_forward = 0;
  int m_turn = 0;
  int m_ErrorPowerLeft = 0;
  int m_ErrorPowerRight = 0;
  int m_ErrorIntergral = 0;
  int m_RightSpeed = 0;
  int m_LeftSpeed = 0;

public:
  static int GetMovementJoystick(unsigned char joystick, unsigned char axis, int minValue);

  int GetForwardAxis();
  int GetTurnAxis();

  void SetLeftDrive(int speed);
  void SetRightDrive(int speed);
  void DebugDrive();

  void Update();
};
