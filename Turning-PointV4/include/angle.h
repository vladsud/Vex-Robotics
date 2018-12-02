#pragma once
#include "main.h"

  enum class Flag
  {
    High,
    Middle,
    Loading,
  };

// Angle potentiometer:
const int anglePotentiometerLow = 1820;
const int anglePotentiometerHigh = 1060;

class Shooter
{
    bool m_fMoving = false;
    int m_angleToMove = anglePotentiometerLow;
    int m_count = 0;
    bool m_Manual = false;

    int m_distanceInches = 72;
    Flag m_flag = Flag::Loading;

    int m_lastAngleDistance = 0;
    int m_diffAdjusted = 0;

  public:
    Shooter();
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