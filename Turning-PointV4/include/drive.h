#pragma once
#include "main.h"

class Drive
{
  float m_forward = 0;
  float m_turn = 0;
  float m_ErrorPower = 0;

public:
  float GetMovementJoystick(unsigned char joystick, unsigned char axis);

  float GetForwardAxis();
  float GetTurnAxis();

  void SetLeftDrive(float speed);
  void SetRightDrive(float speed);

  void Update();
};
