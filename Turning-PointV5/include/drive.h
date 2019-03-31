#pragma once
#include "main.h"

// units - whatever motor_get_actual_velocity() returns (RPM)
double GetLeftVelocity();
double GetRightVelocity();
double GetRobotVelocity();

struct DriveTracker
{
  DriveTracker();
  ~DriveTracker();
  virtual float GetError() { Assert(false); return 0; } // for some reason linker can't find __cxa_pure_virtual

protected:
  class Main& m_main;
  class Drive& m_drive;
};


// All streight movements will follow gyro angle as long as this bject is on thw statck
struct KeepAngle : public DriveTracker
{
  KeepAngle(int angle);
  float GetError() override;
protected:
  int m_angle;
};


class Drive
{
  int m_forward = 0;
  int m_overrideForward = 0;
  int m_encoderBaseLeft = 0;
  int m_encoderBaseRight = 0;
  int m_left = 0;
  int m_right = 0;
  float m_turn = 0;
  float m_overrideTurn = 0;
  float m_ErrorIntergral = 0;
  bool m_flipX = false;
  bool m_holdingPosition = false;

  DriveTracker* m_tracker = nullptr;

  bool KeepDrection(int forward, float turn)
  {
    // Do not reset m_forward & m_turn on full stop!
    // that screws up autonomous turning.
    return (m_turn == 0 && turn == 0 && forward * m_forward >= 0) ||
           (m_forward == 0 && forward == 0) ||
           (m_turn == turn && m_forward == forward);
  }

  void HoldPosition();
  int GetForwardAxis();
  float GetTurnAxis();
  void SetLeftDrive(int speed);
  void SetRightDrive(int speed);
  static int GetMovementJoystick(pros::controller_id_e_t joystick, pros::controller_analog_e_t axis, int minValue);

public:
  unsigned int m_distance = 0;
  // This value does not reset by atonomous or manual operations
  // But, it can decrees (to zero) when robot goes bacwards.
  unsigned int m_distanceFromBeginning = 0;

public:
  Drive();
  void FlipX(bool flip) { m_flipX = flip; }
  bool IsXFlipped() const { return m_flipX; }
  void StartHoldingPosition();

  // Forward: Positive turn - turn right (clockwise)
  // Backwards: Positive turn - turn left (clockwise)
  void OverrideInputs(int forward, float turn);
  void ResetTrackingState();
  void Update();
  void UpdateDistanes();

  void ResetState();

  void StartTracking(DriveTracker* tracker)
  {
    Assert(m_tracker == nullptr);
    m_tracker = tracker;
  }
  void StopTracking()
  {
    Assert(m_tracker != nullptr);
    m_tracker = nullptr;
  }
};
