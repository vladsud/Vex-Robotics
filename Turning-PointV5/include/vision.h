#pragma once
#include "pros/vision.hpp"

enum SigType
{
    Green,
    Blue,
    Red,
    BlueFull,
    RedFull,
};


class Vision
{
    pros::vision_object_s_t m_objects[8];
    int32_t m_objCount = 0;

    pros::Vision m_sensor;
    SigType m_type = SigType::Red;
    unsigned int m_count = 0;
    unsigned int m_detectionsHigh = 0;
    unsigned int m_detectionsLow = 0;
    unsigned int m_countNotFound = 0;
    unsigned int m_countShooterMoving = 0;
    unsigned int m_anglePos = 0;
    unsigned m_brightness = 30;
    unsigned m_trackingX = 30;
    unsigned m_trackingY = 30;
    bool m_reportedError = false;
    bool m_blue = false;
    bool m_fOnTarget = false;
    bool m_isShooting = false;

public:
    Vision();
    void Update();
    void SetFlipX(bool blue); 
    bool OnTarget();
    bool IsShooting();

protected:
    bool ReadObjects();
    bool FindObject(unsigned int xDistanceMax, unsigned yDistanceMax, unsigned int minConfidence, bool moveToIt);
};

