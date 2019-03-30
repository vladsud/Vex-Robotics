#pragma once
#include "main.h"
#include "pros/adi.hpp"

enum class Flag
{
  High,
  Middle,
  Loading,
};

enum class BallPresence
{
  NoBall,
  Unknown,
  HasBall
};

BallPresence BallStatus(pros::ADIAnalogIn& sensor);

class Shooter
{
  pros::ADIAnalogIn m_preloadSensor;
  pros::ADIAnalogIn m_angleSensor;
  pros::ADIAnalogIn m_ballPresenceSensorUp;
  pros::ADIAnalogIn m_ballPresenceSensorDown;

  // This should be comming from autonomous, probably.
  unsigned int m_distanceInches = 48;
  Flag m_flag = Flag::High;

  // it is recalculated in constructor, so value does not matter that much
  unsigned int m_angleToMove = 1500;
  unsigned int m_count = 0;
  unsigned int m_lastAngleDistance = 0;
  int m_diffAdjusted = 0;
  int m_preLoadCount = 0;
  int m_preloadAfterShotCounter = 0;
  unsigned int m_timeSinseShooting = 0;
  unsigned int m_shooterWait = 0;

  bool m_fMoving = false;
  bool m_disablePreload = false;
  bool m_userShooting = false;
  bool m_preloading = false;
  bool m_overrideShooting = false;
  bool m_haveBall = false;
  bool m_haveBall2 = false;

public:
  Shooter();
  void KeepMoving();
  void StartMoving();
  void StopMoving();
  void Update();
  void Debug();
  unsigned int CalcAngle();
  void UpdateDistanceControls();
  void SetDistance(unsigned int distance);
  void SetFlag(Flag flag);

  void OverrideSetShooterMode(bool on);
  bool IsMovingAngle();
  bool IsShooting();
  BallPresence BallStatus();
  Flag GetFlagPosition() { return m_flag; }
};
