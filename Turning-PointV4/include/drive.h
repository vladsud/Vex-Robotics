#pragma once
#include "main.h"

class Drive
{
  int m_forward = 0;
  int m_turn = 0;
  int m_ErrorPower = 0;

public:
  static int GetMovementJoystick(unsigned char joystick, unsigned char axis);

  int GetForwardAxis();
  int GetTurnAxis();

  void SetLeftDrive(int speed);
  void SetRightDrive(int speed);
  void DebugDrive();

  void Update();
};
