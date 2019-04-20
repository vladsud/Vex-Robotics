#include "main.h"
#include"vision.h"
#include "pros/adi.h" // for PROS_ERR
#include "pros/rtos.h"
#include "errno.h"
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "pros/rtos.h"
#include "cycle.h"
#include "pros/motors.h"

 #define RGB2COLOR(R, G, B) ((R & 0xff) << 16 | (G & 0xff) << 8 | (B & 0xff)) 

using namespace pros::c;

struct Signature
{
    SigType type;
    const char* name;
    uint8_t brightness;
    pros::vision_signature_s_t sig;
};

Signature g_signatures[] = {
    {SigType::Green, "Green 2", 30, vision_signature_from_utility(1, -2891, -2245, -2568, -4615, -3847, -4231, 5.500, 0)},
    {SigType::Green, "Green", 50, vision_signature_from_utility(1, -2965, -2365, -2665, -4899, -4239, -4569, 4.500, 0)},
    {SigType::Blue,  "Blue 2", 30, vision_signature_from_utility(1, -3329, -2467, -2898, 12655, 14947, 13801, 4.000, 0)},
    {SigType::Blue,  "Blue", 50, vision_signature_from_utility(1, -3339, -1973, -2656, 11547, 15541, 13544, 3.000, 0)},
//    {SigType::BlueFull, "Blue full", 30, vision_signature_from_utility(1, -3145, -2205, -2675, -1299, 14607, 6654, 1.500, 0)},
};

struct ObjTracker
{
    unsigned int confidence;
    unsigned int distance;
    pros::vision_object_s_t* green;
    pros::vision_object_s_t* mainColor;
};

ObjTracker& FindBestMatch(ObjTracker tracker[], unsigned int tracking)
{
    unsigned int confidenceMax = 0;
    unsigned int bestMatchIndex = 0;

    for (int i = 0; i < tracking; i++)
    {
        auto& obj = tracker[i];
        if (confidenceMax < obj.confidence)
        {
            confidenceMax = obj.confidence;
            bestMatchIndex = i;
        }
    }
    return tracker[bestMatchIndex];
}

bool IsValidObject(pros::vision_object_s_t& obj, SigType type, bool report)
{
    // some m_objects can have m_objects[i].signature == VISION_OBJECT_ERR_SIG if failed to retrieve.
    // Otherwise m_objects[i].signature is either signature ID (1..7) or color code.
    if (obj.signature == VISION_OBJECT_ERR_SIG)
    {
        if (report)
            ReportStatus("Failed to retrive object\n");
        return false;
    }
    auto index = obj.signature;
    if (index <= 0 || index > CountOf(g_signatures))
    {
        // ReportStatus("   Vision: Wrong index: %d\n", index);
        return false;
    }

    auto& sig = g_signatures[index-1];
    if (sig.type != type)
        return false;

    return true;
}

Vision::Vision()
    : m_sensor(VisionPort, pros::E_VISION_ZERO_CENTER)
{
    // testing
    SetFlipX(true);

    m_sensor.set_wifi_mode(0);
    m_sensor.set_led(0x00ffffff);

    m_sensor.set_auto_white_balance(1);
    // m_sensor.set_white_balance(rgb);
}

void Vision::SetFlipX(bool blue) 
{
    // 15 & 60 seems like best spots!
    // 30-35 is best in dark places (60 fine too for one color matching)
    m_brightness = 60;
    m_sensor.set_exposure(m_brightness); // 0..150

    m_blue = blue;
    m_type = blue ? SigType::Blue : SigType::Red;

    // first arg is written into 'id' field
    for (int i = 0; i < CountOf(g_signatures); i++)
    {
        if (PROS_ERR == m_sensor.set_signature(i+1, &g_signatures[i].sig))
            ReportStatus("Faield to set vision signature %d\n", i);
    }
} 

bool Vision::IsShooting()
{
    return joystickGetDigital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_DOWN);
}

bool Vision::OnTarget()
{
    return m_aimingNShooting;
//     return m_fOnTarget && IsShooting();
}

bool Vision::ReadObjects()
{
    m_objCount = m_sensor.read_by_size(0, CountOf(m_objects), m_objects);
    if (m_objCount == 0)
        return false;
    if (m_objCount == PROS_ERR || m_objCount < 0)
    {
        // no objects found
        if (errno != EDOM)
        {
            // EINVAL = 22 - incorrect port or port type
            // EACCES = 13 - someone else is talking to same port
            if (!m_reportedError)
                ReportStatus("Vision sensor error: %d\n", errno);
            m_reportedError = true;
        }
        return false;
    }

    if (m_objCount > CountOf(m_objects))
        m_objCount = CountOf(m_objects);
    // ReportStatus("Vision: %d m_objects: time = %d\n", m_objCount, millis());
    return true;
}

bool Vision::FindObject(unsigned int xDistanceMax, unsigned yDistanceMax, unsigned int minConfidence, bool moveToIt)
{
    ObjTracker tracker[30];
    unsigned int tracking = 0;
    unsigned int minDistanece = UINT_MAX;
    unsigned int minDistanecePair = UINT_MAX;

    for (int i = 0; i < m_objCount && tracking < CountOf(tracker); i++)
    {
        auto& objectMainColor = m_objects[i];

        /*
        if (moveToIt)
            ReportStatus("   obj: id=%d, x=%d, y=%d, w=%d, h=%d, a=%d\n",
                objectMainColor.signature,
                objectMainColor.x_middle_coord,
                objectMainColor.y_middle_coord,
                objectMainColor.width,
                objectMainColor.height,
                objectMainColor.angle);
        */

        if (!IsValidObject(objectMainColor, m_type, true))
            continue;

        if (abs(objectMainColor.x_middle_coord) > xDistanceMax)
            continue;
        if (abs(objectMainColor.y_middle_coord) > yDistanceMax)
            continue;

        auto& currentMainColor = tracker[tracking];
        currentMainColor.mainColor = &objectMainColor;
        currentMainColor.green = nullptr;
        currentMainColor.confidence = 1;
        currentMainColor.distance = objectMainColor.x_middle_coord * objectMainColor.x_middle_coord + objectMainColor.y_middle_coord * objectMainColor.y_middle_coord;
        if (currentMainColor.distance < minDistanece)
            minDistanece = currentMainColor.distance;
        tracking++;

        for (int j = 0; j < m_objCount && tracking < CountOf(tracker); j++)
        {
            auto& object = m_objects[j];
            if (!IsValidObject(object, SigType::Green, false))
                continue;

            auto& current = tracker[tracking];
            current.mainColor = &objectMainColor;
            current.green = &object;
            current.distance = currentMainColor.distance;
            current.confidence = currentMainColor.confidence;
            if (currentMainColor.distance < minDistanecePair)
                minDistanecePair = currentMainColor.distance;

            float heightRatio = (float)object.height / objectMainColor.height;
            int horizonalDiff;
            if (!m_blue)
                horizonalDiff = object.left_coord - objectMainColor.left_coord - objectMainColor.width;
            else
                horizonalDiff = object.left_coord + object.width - objectMainColor.left_coord;
            int vertDiff = abs(object.y_middle_coord - objectMainColor.y_middle_coord);

            current.confidence += 1; // some pair
            if (abs(horizonalDiff) < objectMainColor.width)
                current.confidence += 4;
            if (vertDiff < objectMainColor.height * 2 || vertDiff <= 8)
                current.confidence += 8;
            if (heightRatio < 4 && heightRatio > 0.25)
                current.confidence += 2;

            tracking++;
        }
    }

    Assert(tracking < CountOf(tracker));

    if (tracking == 0)
        return false;

    // Find closest
    for (int i = 0; i < tracking; i++)
    {
        auto& obj = tracker[i];
        if (obj.distance == minDistanece)
            obj.confidence += 3;
        else if (obj.green != nullptr && obj.distance == minDistanecePair)
            obj.confidence += 3;
    }

    auto& obj = FindBestMatch(tracker, tracking);
    if (obj.confidence < minConfidence)
        return false;

    int x = obj.mainColor->x_middle_coord;
    int y = - obj.mainColor->y_middle_coord; // pos coordinate is down
    m_fOnTarget = abs(x) <= 5 && abs(y) <= 10;
    /*
    if ((m_m_objCount % 10) == 0)
        ReportStatus("(%3d) Vision best match: confidence=%2d coord=(%3d, %3d), size=(%3d, %3d)\n",
            m_brightness,
            obj.confidence,
            obj.mainColor->x_middle_coord,
            obj.mainColor->y_middle_coord,
            obj.mainColor->width,
            obj.mainColor->height);
    */
    if (moveToIt)
    {
        int y_angle = y;
        if (abs(y) > 50)
            y_angle = y_angle * 3.5;
        else
            y_angle = y_angle * 3;

        auto& main = GetMain();
        ReportStatus("Tracking: confidence = %d, width = %d, coord = %d %d, angle diff = %d, %d, angle: %d\n",
            obj.confidence, obj.mainColor->width, x, y, y_angle, main.shooter.MovingRelativeTo(), (int)motor_get_position(angleMotorPort));

        if (abs(x) > 5)
        {
            if (x > 8)
                x = 8;
            else if (x < -8)
                x = -8;
            x = x * 2 + Sign(x)*10;
            main.drive.OverrideInputs(0, x); // positive ia turn right (clockwise)
        }
        else
        {
            main.drive.OverrideInputs(0, 0);
        }
        
        int y_expected = main.shooter.MovingRelativeTo();
        if (abs(y) > 8 || abs(y_expected) > 20)
        {
            m_countShooterMoving++;
            // y_angle += Sign(y_angle) * 20;
            if (y_expected == 0 || m_countShooterMoving >= 10) // it is moving
            {
//                y_angle = (y_angle + 15 * y_expected) / 16;
                main.shooter.MoveAngleRelative(y_angle);
                m_countShooterMoving = 0;
            }            
        }
        else
        {
            main.shooter.MoveAngleRelative(0);
            m_countShooterMoving = 0;
        }
    }
    return true;
}

void Vision::Update()
{
    m_count++;
    auto& main = GetMain();

    // Calibration code
    if (false)
    {
        if ((m_count % 100) == 0)
        {
            ReportStatus("(%3d) detections: %3d / %3d\n", m_brightness, m_detectionsHigh, m_detectionsHigh + m_detectionsLow);
            m_detectionsHigh = 0;
            m_detectionsLow = 0;
            m_brightness += 5;
            if (m_brightness == 150)
                m_brightness = 0;
            m_sensor.set_exposure(m_brightness); // 0..150
        }
        if (FindObject(30, 30, 17, false))
            m_detectionsHigh++;
        else if (FindObject(30, 30, 11, false))
            m_detectionsLow++;
        return;
    }

    bool found = false;
    bool move = IsShooting();
    if (ReadObjects())
    {
        for (int i = 0; i < m_objCount; i++)
        {
            auto& obj = m_objects[i];
            obj.y_middle_coord += 45;
        }

        found =
            FindObject(30, 30, 17, move) ||
            FindObject(100, 100, 17, move) ||
            FindObject(30, 30, 11, move) ||
            FindObject(30, 30, 4, move);
        if (!found)
            m_countNotFound++;
        if (m_countNotFound >= 5)
        {
            m_fOnTarget = false;
            m_countNotFound = 0;
        }
        
        if (!move)
            m_fOnTarget = false;
    }

    bool newValue = m_fOnTarget && move;
    if (!newValue && m_aimingNShooting)
    {
        main.drive.OverrideInputs(0, 0);
        main.shooter.MoveAngleRelative(0);
        m_countShooterMoving = 0;
    }
    m_aimingNShooting = newValue;

    if (move && !found)
        ReportStatus("not found\n");
}
