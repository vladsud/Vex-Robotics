#pragma once
#include "main.h"

class Drive
{
  float forward = 0;
  float turn = 0;
  float ErrorPower = 0;

public:
  float GetMovementJoystick(unsigned char joystick, unsigned char axis);

  float GetForwardAxis();
  float GetTurnAxis();

  void SetLeftDrive(float speed);
  void SetRightDrive(float speed);

  void Update();
};
