#pragma once
#include "main.h"

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

class Shooter
{
  // This should be comming from autonomous, probably.
  unsigned int m_distanceInches = 48;
  Flag m_flag = Flag::Loading;

  // it is recalculated in constructor, so value does not matter that much
  unsigned int m_angleToMove = 0;
  unsigned int m_count = 0;
  unsigned int m_lastAngleDistance = 0;
  int m_diffAdjusted = 0;
  int m_preLoadCount = 0;
  int m_preloadAfterShotCounter = 0;
  unsigned int m_timeSinseShooting = 0;

  bool m_fMoving = false;
  bool m_Manual = false;
  bool m_disablePreload = false;
  bool m_userShooting = false;
  bool m_preloading = false;
  bool m_overrideShooting = false;

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
  bool IsMovingAngle() { return m_fMoving; }
  bool IsShooting();
  BallPresence BallStatus();
  Flag GetFlagPosition() { return m_flag; }
};