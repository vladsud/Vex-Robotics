#include "main.h"
#include "pros/vision.hpp"
#include "pros/adi.h" // for PROS_ERR
#include "errno.h"
#include <stdio.h>

#define VisionPort 1
#define RGB2COLOR(R, G, B) ((R & 0xff) << 16 | (G & 0xff) << 8 | (B & 0xff)) 

using namespace pros::c;

struct Signature
{
    const char* name;
    uint8_t brightness;
    pros::vision_signature_s_t sig;
};

Signature g_signatures[] = {
    {"Green", 30, vision_signature_from_utility(1, -2891, -2245, -2568, -4615, -3847, -4231, 6.000, 0)},
    {"Green", 50, vision_signature_from_utility(1, -2965, -2365, -2665, -4899, -4239, -4569, 5.000, 0)},
    {"Green", 80, vision_signature_from_utility(1, -785, -47, -416, -2743, -687, -1715, 1.500, 0)},
    {"Blue", 30, vision_signature_from_utility(1, -3329, -2467, -2898, 12655, 14947, 13801, 3.500, 0)}, // pretty wide detection
    {"Blue", 50, vision_signature_from_utility(1, -3339, -1973, -2656, 11547, 15541, 13544, 2.000, 0)},
//    {"Blue", 80, vision_signature_from_utility(1, -4083, -2645, -3364, 7959, 14137, 11048, 1.000, 0)}, // really bad detection
    {"Blue full", 30, vision_signature_from_utility(1, -3145, -2205, -2675, -1299, 14607, 6654, 2.100, 0)},
    {"Blue full", 50, vision_signature_from_utility(1, -3223, -2311, -2767, -1763, 14641, 6439, 2.000, 0)},
//    {"Blue full", 80, vision_signature_from_utility(1, -3595, -47, -1821, -317, 12965, 6324, 1.300, 0)}, // can't detect at all
};

class Vision
{
    pros::Vision m_sensor;
    unsigned int m_count = 0;

public:
    Vision();
    void Update();
};


Vision::Vision()
    : m_sensor(VisionPort, pros::E_VISION_ZERO_CENTER)
{
    m_sensor.set_wifi_mode(0);
    m_sensor.set_led(0x00ffffff);
    m_sensor.set_exposure(50); // 0..150

    m_sensor.set_auto_white_balance(1);
    // m_sensor.set_white_balance(rgb);

    // first arg is written into 'id' field
    for (int i = 0; i < CountOf(g_signatures); i++)
    {
        if (PROS_ERR == m_sensor.set_signature(i+1, &g_signatures[i].sig))
            ReportStatus("Faield to set vision signature %d\n", i);
    }
}

void Vision::Update()
{
    m_count++;
    if ((m_count % 50) != 0)
        return;

    // Objects, by size.
    pros::vision_object_s_t objects[8];
    int32_t count = m_sensor.read_by_size(0, CountOf(objects), objects);
    if (count == 0)
        return;
    if (count == PROS_ERR)
    {
        // EINVAL = 22 - incorrect port or port type
        // EACCES = 13 - someone else is talking to same port
        // EDOM (33?) - first arg is bigger then number of objects.
        ReportStatus("Vision sensor error: %d\n", errno);
        return;
    }

    // some objects can have objects[i].signature == VISION_OBJECT_ERR_SIG if failed to retrieve.
    // Otherwise objects[i].signature is either signature ID (1..7) or color code.
    if (count > CountOf(objects))
    {
        Assert(false);
        count = CountOf(objects);
    }

    for (int i = 0; i < count; i++)
    {
        if (objects[i].signature == VISION_OBJECT_ERR_SIG)
        {
            ReportStatus("Failed to retrive object\n");
        }
        else
        {
            auto& obj = g_signatures[objects[i].signature-1];
            ReportStatus("Vision object detected: %s %d, coord=(%d, %d), size=(%d, %d)\n",                    
                    obj.name,
                    obj.brightness,
                    objects[i].x_middle_coord, objects[i].y_middle_coord,
                    objects[i].width, objects[i].height);
        }
    }
}
