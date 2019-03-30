#include "main.h"
#include "pros/vision.hpp"
#include "pros/adi.h" // for PROS_ERR
#include "errno.h"
#include <stdio.h>

#define VisionPort 1
#define RGB2COLOR(R, G, B) ((R & 0xff) << 16 | (G & 0xff) << 8 | (B & 0xff)) 

class Vision
{
    pros::Vision m_sensor;
    pros::vision_signature_s_t m_signatures[1] = {};
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
    m_sensor.set_exposure(150); // 0..150

    m_sensor.set_auto_white_balance(1);
    // m_sensor.set_white_balance(rgb);

    // first arg is written into 'id' field
    for (int i = 0; i < CountOf(m_signatures); i++)
        m_sensor.set_signature(i+1, &m_signatures[i]);
}

void Vision::Update()
{
    m_count++;
    if ((m_count % 100) != 0)
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
            ReportStatus("Object id=%d, coord=(%d, %d), size=(%d, %d)\n",
                    objects[i].signature,
                    objects[i].x_middle_coord, objects[i].y_middle_coord,
                    objects[i].width, objects[i].height);
        }
    }
}
