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
    unsigned int m_detections = 0;
    unsigned m_brightness = 30;
    bool m_reportedError = false;
    bool m_blue = false;

public:
    Vision();
    void Update();
    void SetFlipX(bool blue); 

protected:
    bool ReadObjects();
    bool FindObject(unsigned int xDistanceMax, unsigned yDistanceMax, unsigned int minConfidence, bool moveToIt);
};

