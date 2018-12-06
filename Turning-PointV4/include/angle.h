#pragma once
#include "main.h"

enum class Flag
{
  High,
  Middle,
  Loading,
};

class Shooter
{
    // This should be comming from autonomous, probably.
    unsigned int m_distanceInches = 72;
    Flag m_flag = Flag::Loading;

    // it is recalculated in constructor, so value does not matter that much
    unsigned int m_angleToMove = 0;

    bool m_fMoving = false;
    unsigned int m_count = 0;
    bool m_Manual = false;

    unsigned int m_lastAngleDistance = 0;
    int m_diffAdjusted = 0;
    int m_preLoadCount = 0;
    bool m_disablePreload = false;
    bool m_userShooting = false;
    bool m_preloading = false;
    bool m_preloadAfterShot = false;

  public:
    Shooter();
    bool GetAngleDown();
    void KeepMoving();
    void StartMoving();
    void StopMoving();
    void Update();
    void Debug();
    unsigned int  CalcAngle();
    void UpdateDistance();
    void SetDistance(unsigned int distance);
    void SetFlag(Flag flag);

    bool IsMoving() { return m_fMoving; }
};