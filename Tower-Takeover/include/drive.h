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

class Drive
{
  int m_forward = 0;

  int m_overrideForward = 0;
  int m_overrideTurn = 0;

  int m_turn = 0;
  float m_ErrorIntergral = 0;
  bool m_flipX = false;

  int m_encoderLeftBase = 0;
  int m_encoderRightBase = 0;

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
  Drive();
  void FlipX(bool flip) { m_flipX = flip; }
  bool IsXFlipped() const { return m_flipX; }

  // Forward: Positive turn - turn right (clockwise)
  // Backwards: Positive turn - turn left (clockwise)
  void OverrideInputs(int forward, int turn);
  void ResetTrackingState();
  void Update();

  void ResetState();
  int GetDistance();
  int GetLeft();
  int GetRight();
  
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
