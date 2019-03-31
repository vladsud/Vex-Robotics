#include "pros/vision.hpp"

class Vision
{
    pros::Vision m_sensor;
    unsigned int m_count = 0;
    bool m_reportedError = false;

public:
    Vision();
    void Update();
};

