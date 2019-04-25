//vision::signature SIG_1 (1, -2377, -953, -1665, 9243, 13343, 11293, 2.500, 0);
//blue flag
//vision::signature SIG_2 (2, -3447, -1273, -2360, -6387, -409, -3398, 1.100, 0);
//green spot
//vision::signature SIG_3 (3, 0, 0, 0, 0, 0, 0, 3.000, 0);
//vision::signature SIG_4 (4, 0, 0, 0, 0, 0, 0, 3.000, 0);
//vision::signature SIG_5 (5, 0, 0, 0, 0, 0, 0, 3.000, 0);
//vision::signature SIG_6 (6, 0, 0, 0, 0, 0, 0, 3.000, 0);
//vision::signature SIG_7 (7, 0, 0, 0, 0, 0, 0, 3.000, 0);
//vex::vision vision1 ( vex::PORT1, 20, SIG_1, SIG_2, SIG_3, SIG_4, SIG_5, SIG_6, SIG_7 );


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
#include "ActionsTurn.h"

 #define RGB2COLOR(R, G, B) ((R & 0xff) << 16 | (G & 0xff) << 8 | (B & 0xff)) 

const unsigned int maxX = 200; // we do not want it to turn too far!
const unsigned int maxY = 300;


using namespace pros::c;

struct Signature
{
    SigType type;
    const char* name;
    uint8_t brightness;
    pros::vision_signature_s_t sig;
};

Signature g_signatures[] = {
    // Laps in bonus roon
    {SigType::Green, "Green 1", 30, vision_signature_from_utility(3, -3515, -2543, -3029, -5301, -4029, -4665, 4.100, 0)},
    {SigType::Green, "Green 2", 60, vision_signature_from_utility(4, -3515, -2543, -3029, -5301, -4029, -4665, 6.000, 0)},
    {SigType::Red, "Red 1", 60, vision_signature_from_utility(1, 5795, 9547, 7670, -1181, 255, -464, 3.000, 0)},
    {SigType::Red, "Red 2", 30, vision_signature_from_utility(2, 6275, 9623, 7949, -1159, 269, -445, 3.500, 0)},
    {SigType::Blue, "Blue 1", 60, vision_signature_from_utility(1, -4229, -2233, -3232, 9557, 15711, 12634, 3.000, 0)},
    {SigType::Blue, "Blue 2", 30, vision_signature_from_utility(2, -4255, -2021, -3138, 8995, 15745, 12370, 2.700, 0)}

    // Natural light
    /*
    {SigType::Green, "Green 2", 30, vision_signature_from_utility(1, -2891, -2245, -2568, -4615, -3847, -4231, 7.500, 0)},
    {SigType::Green, "Green", 50, vision_signature_from_utility(1, -2965, -2365, -2665, -4899, -4239, -4569, 6.500, 0)},
    {SigType::Blue,  "Blue 2", 30, vision_signature_from_utility(1, -3329, -2467, -2898, 12655, 14947, 13801, 6.000, 0)},
    {SigType::Blue,  "Blue", 50, vision_signature_from_utility(1, -3339, -1973, -2656, 11547, 15541, 13544, 5.000, 0)},
    */
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
    // by default, starting as red
    SetFlipX(false);

    m_sensor.set_wifi_mode(0);
    m_sensor.set_led(0x00ffffff);

    m_sensor.set_auto_white_balance(1);
    // m_sensor.set_white_balance(rgb);
}

void Vision::SetFlipX(bool blue) 
{
    // If we are playing blue, we need to find red flags!
    blue = !blue;

    m_brightness = 60;
    m_sensor.set_exposure(m_brightness); // 0..150

    m_blue = blue;
    m_type = blue ? SigType::Blue : SigType::Red;

    // first arg is written into 'id' field
    for (int i = 0; i < CountOf(g_signatures); i++)
    {
        if (1 != m_sensor.set_signature(i+1, &g_signatures[i].sig))
            ReportStatus("Faield to set vision signature %d\n", i);
    }
    ReportStatus("Camera is initialized\n");
} 

bool Vision::IsShooting()
{
    return m_isShootingMoveBase || m_isShootingMoveAngle;
}

bool Vision::OnTarget()
{
    return m_fOnTarget && (m_isShootingMoveBase || m_isShootingMoveAngle);
}

bool Vision::ReadObjects()
{
    m_objCount = m_sensor.read_by_size(0, CountOf(m_objects), m_objects);
    if (m_objCount == 0)
        return false;
    if (m_objCount == PROS_ERR || m_objCount < 0)
    {
        m_objCount = 0;
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

bool Vision::FindObject(unsigned int xDistanceMax, unsigned yDistanceMax, unsigned int minConfidence, bool moveBase, bool moveAngle)
{
    ObjTracker tracker[30];
    unsigned int tracking = 0;
    unsigned int minDistanece = UINT_MAX;
    unsigned int minDistanecePair = UINT_MAX;

    for (int i = 0; i < m_objCount && tracking < CountOf(tracker); i++)
    {
        auto& objectMainColor = m_objects[i];

        /*
        if (moveBase || moveAngle)
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
        currentMainColor.mainColor = &m_objects[i];
        currentMainColor.green = nullptr;
        currentMainColor.confidence = 0;
        // m,ain color part is almost square
        if (objectMainColor.width < objectMainColor.height * 3 / 2 && objectMainColor.height < objectMainColor.width * 3 / 2)
            currentMainColor.confidence += 10; // avoid shooting already toggled flag
        if (objectMainColor.width >= 10)
            currentMainColor.confidence += 10;
        // It's important to prioritise big objects, as we see toggled flag sometimes.
        if (objectMainColor.width >= 20)
            currentMainColor.confidence += 8;
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
            current.mainColor = &m_objects[i];
            current.green = &m_objects[j];
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
    int y = obj.mainColor->y_middle_coord; // pos coordinate is down

    if (m_blue)
        x -= obj.mainColor->width / 4;
    else
        x += obj.mainColor->width / 4;

    if (moveBase || moveAngle)
    {
        m_trackingX = x;
        m_trackingY = y;
    }

    m_fOnTarget = false;
    if (moveBase || moveAngle)
        m_fOnTarget = true;
    if (moveBase && abs(x) >= 5)
        m_fOnTarget = false;
    if (moveAngle && abs(y) > 7)
        m_fOnTarget = false;

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

    if (moveBase || moveAngle)
    {
        int y_angle = - y;
        if (abs(y) > 50)
            y_angle = y_angle * 3.5;
        else
            y_angle = y_angle * 3;

        auto& main = GetMain();
        ReportStatus("Tracking: confidence = %d, dimentions = (%d, %d) coord = (%d %d), angle diff = (%d, %d), angle: %d, angle diff: %d\n",
            obj.confidence, obj.mainColor->width, obj.mainColor->height, x, y, y_angle, main.shooter.MovingRelativeTo(), (int)motor_get_position(angleMotorPort),
            m_turnAction ? m_turnAction->GetError() : 0);

        if (moveBase)
        {
            int angle = -x * GyroWrapper::Multiplier / 4;
            if (!m_turnAction)
                m_turnAction = new TurnPrecise(angle);
            m_turnAction->ChangeTurn(angle);
            (void)m_turnAction->ShouldStop();
        }
        
        if (moveAngle)
        {
            int y_expected = main.shooter.MovingRelativeTo();
            if (abs(y) > 6 || abs(y_expected) > 20)
            {
                if (y_expected == 0 || m_countShooterMoving >= 10) // it is moving
                {
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
    }
    
    return true;
}

void Vision::LostBall()
{
    // Not look at flags for a while...
    // First, the flag takes a while to flip and for camera to notice it
    // By waiting, we allow shooter default behaviour to start moving to another flag and actually start noticing it
    m_lostBallCount = 40;
    m_fOnTarget = false;
    if (m_isShootingMoveBase)
        StopTurning();
    m_countShooterMoving = 0;
    m_trackingX = maxX;
    m_trackingY = maxY;

    if (isAuto())
        ShootingInAutonomous(false, false);
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
            ReportStatus("Brightness = %3d, detections (high / medium / low): %3d / %3d / %3d == %3d\n",
                m_brightness,
                m_detectionsHigh, m_detectionsMedium, m_detectionsLow,
                m_detectionsHigh + m_detectionsMedium + m_detectionsLow);
            m_detectionsHigh = 0;
            m_detectionsMedium = 0;
            m_detectionsLow = 0;
            m_brightness += 5;
            if (m_brightness == 150)
                m_brightness = 0;
            m_sensor.set_exposure(m_brightness); // 0..150
        }
        if (ReadObjects())
        {
            if (FindObject(maxX, maxY, 32, false, false))
                m_detectionsHigh++;
            else if (FindObject(maxX, maxY, 28, false, false))
                m_detectionsMedium++;
            else if (FindObject(maxX, maxY, 20, false, false))
                m_detectionsLow++;
        }
        return;
    }

    bool found = false;
    bool moveBase = joystickGetDigital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_R1);
    bool moveAngle = moveBase;

    // In auton, just leverage
    if (isAuto())
    {
        moveBase = m_isShootingMoveBase;
        moveAngle = m_isShootingMoveAngle;
    }

    // Ignore everything after the shot - let the shooter move to other ball and us start to capture it.
    if (m_lostBallCount > 0)
    {
        m_lostBallCount--;
    }
    else
    {
        if (moveAngle)
        {
            m_countShooterMoving++;
            m_isShootingMoveAngle = moveAngle;
        }
        if (moveBase)
            m_isShootingMoveBase = moveBase;

        if (ReadObjects())
        {
            // Camera does not look precisely streight, and the ball does not fly on streight line
            // So need to adjust for that by "moving" all objects
            for (int i = 0; i < m_objCount; i++)
                m_objects[i].y_middle_coord += 70;

            // resolution: 640 x 400
            // field-of-view: 75 degrees horizontal, 47 degrees vertical
            // so roughly 8 x 8 pixels is 1 degree in each dimention.
            // 26 - max
            // 22 - next best thing
            found =
                FindObject(abs(int(m_trackingX)) + 20, abs(int(m_trackingX)) + 20, 11, moveBase, moveAngle) ||
                FindObject(maxX, maxY, 11, moveBase, moveAngle);
            }

        if (!found)
        {
            m_countNotFound++;
            if (m_countNotFound >= 10)
            {
                ReportStatus("Stopping vision motion\n");
                m_fOnTarget = false;
                m_countNotFound = 0;
                StopTurning();
            }
        }
        else
            m_countNotFound = 0;

        if (moveBase || moveAngle)
        {
            if (found)
                m_foundCount++;
            if (!found)
                ReportStatus("not found (objects inspected: %d)\n", m_objCount);
        }
    }

    ChangeState(moveBase, moveAngle);
}

void Vision::ChangeState(bool moveBase, bool moveAngle)
{
    auto& main = GetMain();
    if (!moveBase && m_isShootingMoveBase)
        StopTurning();
    if (!moveAngle && m_isShootingMoveAngle)
    {
        main.shooter.MoveAngleRelative(0);
        m_countShooterMoving = 0;
    }
    if (!moveBase && !moveAngle)
    {
        m_trackingX = 400;
        m_trackingY = 300;
        m_foundCount = 0;
        m_countNotFound = 0;
    }

    m_isShootingMoveBase = moveBase;
    m_isShootingMoveAngle = moveAngle;
}

void Vision::StopTurning()
{
    if (m_turnAction)
    {
        m_turnAction->Stop();
        delete m_turnAction;
        m_turnAction = nullptr;
    }
    GetMain().drive.OverrideInputs(0, 0);
}

void Vision::ShootingInAutonomous(bool visionMove, bool visionAngle)
{
    Assert(isAuto());
    ChangeState(visionMove, visionAngle);
}

unsigned int Vision::GetAndResetFoundCount()
{
    unsigned int result = m_foundCount;
    m_foundCount = 0;
    return result;
}
