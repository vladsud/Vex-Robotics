#pragma once
#include "main.h"

class Drive
{
  int m_forward = 0;
  int m_overrideForward = 0;
  int m_encoderBaseLeft = 0;
  int m_encoderBaseRight = 0;
  int m_gyro = 0;
  float m_turn = 0;
  float m_overrideTurn = 0;
  float m_ErrorIntergral = 0;
  bool m_flipX = false;
  bool m_holdingPosition = false;
  bool m_trackAngle = false;

  bool KeepDrection(int forward, float turn)
  {
    // Do not reset m_forward & m_turn on full stop!
    // that screws up autonomous turning.
    return
        (m_turn == 0 && turn == 0 && forward * m_forward >= 0) ||
        (m_forward == 0 && forward == 0) ||
        (m_turn == turn && m_forward == forward);
  }

  void HoldPosition();
  int GetForwardAxis();
  float GetTurnAxis();
  void SetLeftDrive(int speed);
  void SetRightDrive(int speed);
  static int GetMovementJoystick(unsigned char joystick, unsigned char axis, int minValue);
  void ResetTrackingStateCore();

public:
  unsigned int m_distance = 0;
  // This value does not reset by atonomous or manual operations
  // But, it can decrees (to zero) when robot goes bacwards.
  unsigned int m_distanceFromBeginning = 0;

public:
  Drive() { ResetTrackingState(); }
  void FlipX(bool flip) { m_flipX = flip; }
  void StartHoldingPosition();

  // Forward: Positive turn - turn right (clockwise)
  // Backwards: Positive turn - turn left (clockwise)
  void OverrideInputs(int forward, float turn);
  void ResetTrackingState();
  void Update();

  void StartTrackingAngle(int angle);
  void StopTrackingAngle();
  void ResetState();
};
