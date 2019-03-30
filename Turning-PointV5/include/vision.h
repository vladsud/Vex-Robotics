#include "pros/vision.hpp"

class Vision
{
    pros::Vision m_sensor;
    unsigned int m_count = 0;

public:
    Vision();
    void Update();
};

