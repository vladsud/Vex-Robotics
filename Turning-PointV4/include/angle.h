#pragma once
#include "main.h"

  enum class Flag
  {
    High,
    Middle,
    Loading,
  };

class Angle
{
    bool m_fMoving = false;
    int m_angleToMove = 0;
    int m_count = 0;

    int m_distanceInches = 48;
    // it's better to have non-loading default, as changing either flag or
    // distance would properly change angle, and clicking Loading would work.
    Flag m_flag = Flag::Middle;

    int m_lastAngleDistance = 0;
    float m_diffAdjusted = 0;

  public:
    Angle();
    bool GetAngleDown();
    void KeepMoving();
    void StartMoving(int destination);
    void StopMoving();
    void Update();
    void Debug();
    int  CalcAngle();
    void UpdateDistance();
    void SetDistance(int distance);
    void SetFlag(Flag flag);
};