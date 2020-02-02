#pragma once

#include "main.h"

struct DriveTracker
{
  DriveTracker();
  ~DriveTracker();
  virtual float GetError() = 0;

protected:
  class Drive& m_drive;
};


// All streight movements will follow gyro angle as long as this object is on the stack
struct KeepAngle : public DriveTracker
{
  KeepAngle(int angle);
  float GetError() override;
protected:
  int m_angle;
};

class Motor
{
public:
  Motor(unsigned int port);
  // Resets "base" of the motor.
  // GetPos() & GetRealTimePos() will start counting from current position,
  // while GetRawPos() is not affected by this call.
  void Reset();
  // Similar to Reset(), but resets motor for all usages:
  // All external accesses to this motor will see current pos being reset
  // GetRawPos() wills start counting from zero.
  void HardReset();
  // Update motor position - affects GetVelocity(), GetPos() & GetRawPos() results
  void Update();
  // Get velocity of motor (in between two Update() calls)
  int GetVelocity() { return m_currValue - m_prevValue; }
  // Returns cached position
  int GetPos() { return m_currValue - m_base; }
  // retrns uncached position
  int GetRealTimePos();
  // returns cached raw position
  int GetRawPos() { return m_currValue; }

private:
  const unsigned int m_port;
  int m_prevValue;
  int m_currValue;
  int m_base;
};

class Drive
{
  Motor m_motorLeftFront;
  Motor m_motorLeftBack;
  Motor m_motorRightFront;
  Motor m_motorRightBack;
  int m_distance = 0;
  int m_forward = 0;
  int m_overrideForward = 0;
  int m_left = 0;
  int m_right = 0;
  int m_turn = 0;
  int m_overrideTurn = 0;
  float m_ErrorIntergral = 0;
  bool m_flipX = false;

  DriveTracker* m_tracker = nullptr;

  bool KeepDrection(int forward, float turn)
  {
    return (m_turn == 0 && turn == 0 && forward * m_forward >= 0) ||
           (m_forward == 0 && forward == 0) ||
           (m_turn == turn && m_forward == forward);
  }

  int GetForward();
  int GetForwardLeftAxis();
  int GetForwardRightAxis();
  int GetTurnAxis();
  void SetLeftDrive(int speed);
  void SetRightDrive(int speed);

public:
  int GetDistance() { return m_distance; }
  int GetRealTimeDistance();

public:
  Drive();
  void FlipX(bool flip) { m_flipX = flip; }
  bool IsXFlipped() const { return m_flipX; }
  int GetAngle();

  // Forward: Positive turn - turn right (clockwise)
  // Backwards: Positive turn - turn left (clockwise)
  void OverrideInputs(int forward, int turn);
  void ResetTrackingState();
  void UpdateOdometry();
  void Update();

  void ResetState();

  int GetFrontVelocity();
  int GetBackVelocity();
  int GetRobotVelocity();

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

Drive& GetDrive();
