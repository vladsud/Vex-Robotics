#include "angle.h"


// Distance based on front of robot
constexpr unsigned int Distances[]    {   24 ,  30,  48,   54,    108};
//   0: flat - loading
// 156: highest angle we can do (roughly 60 degrees)
// At value 72 (roughly 30 degree angle), +1 value results in ~1/3" shift on target, assuming 4th position (54" from target)
constexpr unsigned int AnglesHigh[]   {  136,   104,  78,   74,   60};
constexpr unsigned int AnglesMedium[] {   92,   80,   47,   46,   44};

constexpr unsigned int LastDistanceCount = CountOf(Distances)-1;

constexpr unsigned int ConvertAngleToPotentiometer(unsigned int angle)
{
    return 1900 - angle*5;
}

// Angle potentiometer:
constexpr unsigned int anglerLow = 16; 
const unsigned int anglePotentiometerLow = ConvertAngleToPotentiometer(anglerLow);
const unsigned int anglePotentiometerHigh = 1060;

static_assert(CountOf(Distances) == CountOf(AnglesHigh));
static_assert(CountOf(Distances) == CountOf(AnglesMedium));

static_assert(ConvertAngleToPotentiometer(AnglesMedium[0]) < anglePotentiometerLow);
static_assert(ConvertAngleToPotentiometer(AnglesHigh[LastDistanceCount]) >= anglePotentiometerHigh);

static_assert(Distances[0] < Distances[1]);
static_assert(Distances[1] < Distances[2]);
static_assert(Distances[2] < Distances[3]);
static_assert(Distances[3] < Distances[4]);

static_assert(AnglesHigh[0] >= AnglesHigh[1]);
static_assert(AnglesHigh[1] >= AnglesHigh[2]);
static_assert(AnglesHigh[2] >= AnglesHigh[3]);
static_assert(AnglesHigh[3] >= AnglesHigh[4]);

static_assert(AnglesMedium[0] >= AnglesMedium[1]);
static_assert(AnglesMedium[1] >= AnglesMedium[2]);
static_assert(AnglesMedium[2] >= AnglesMedium[3]);
static_assert(AnglesMedium[3] >= AnglesMedium[4]);

static_assert(AnglesMedium[0] < AnglesHigh[0]);
static_assert(AnglesMedium[1] < AnglesHigh[1]);
static_assert(AnglesMedium[2] < AnglesHigh[2]);
static_assert(AnglesMedium[3] < AnglesHigh[3]);
static_assert(AnglesMedium[4] < AnglesHigh[4]);


constexpr unsigned int CalcAngle(Flag flag, unsigned int distanceInches)
{
    if (flag == Flag::Loading)
        return anglerLow;

    const unsigned int* angles = (flag == Flag::High) ? AnglesHigh : AnglesMedium;
    unsigned int count = CountOf(Distances);

    if (Distances[0] >= distanceInches)
        return angles[0];

    for (unsigned int i = 0; i < count; i++)
    {
        if (Distances[i] >= distanceInches)
        {
            return angles[i-1] - (distanceInches - Distances[i-1]) * (angles[i-1] - angles[i]) / (Distances[i] - Distances[i-1]);
        }
    }

    return angles[count-1];
}

constexpr bool AlmostSameAngle(unsigned int angle1, unsigned int angle2)
{
    return abs((int)angle1-(int)angle2) <= 1;
}

static_assert(CalcAngle(Flag::Loading, Distances[2]) == anglerLow);

static_assert(CalcAngle(Flag::High, Distances[0]-5) == AnglesHigh[0]);
static_assert(CalcAngle(Flag::High, Distances[0]) == AnglesHigh[0]);
static_assert(CalcAngle(Flag::High, Distances[2]) == AnglesHigh[2]);
static_assert(CalcAngle(Flag::High, Distances[LastDistanceCount]) == AnglesHigh[LastDistanceCount]);
static_assert(CalcAngle(Flag::High, Distances[LastDistanceCount]+100) == AnglesHigh[LastDistanceCount]);
static_assert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[2] + Distances[3]) / 2),   (AnglesHigh[2]   + AnglesHigh[3]) / 2));
static_assert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[2]*3 + Distances[3]) / 4), (AnglesHigh[2]*3 + AnglesHigh[3]) / 4));

static_assert(CalcAngle(Flag::Middle, Distances[0]-5) == AnglesMedium[0]);
static_assert(CalcAngle(Flag::Middle, Distances[0]) == AnglesMedium[0]);
static_assert(CalcAngle(Flag::Middle, Distances[2]) == AnglesMedium[2]);
static_assert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]) == AnglesMedium[LastDistanceCount]);
static_assert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]+100) == AnglesMedium[LastDistanceCount]);
static_assert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[2]   + Distances[3]) / 2), (AnglesMedium[2]   + AnglesMedium[3]) / 2));
static_assert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[2]*3 + Distances[3]) / 4), (AnglesMedium[2]*3 + AnglesMedium[3]) / 4));

// Need to figure out initial position
Shooter::Shooter()
{
    StartMoving();

    /*
    unsigned int precision = 10;
    unsigned int angle = analogRead(anglePotPort);
    
    if (abs(angle - anglePotentiometerLow) < precision)
    {
        m_flag = Flag::Loading;
        return;
    }

    for (unsigned int i = 0; i < CountOf(Distances); i++)
    {
        if (abs(angle - ConvertAngleToPotentiometer(AnglesHigh[i]) < precision)
        {
            m_flag = Flag::High;
            m_distanceInches = Distances[i];
            return;
        }
    }

    for (unsigned int i = 0; i < CountOf(Distances); i++)
    {
        if (abs(angle - ConvertAngleToPotentiometer(AnglesMedium[i]) < precision)
        {
            m_flag = Flag::Middle;
            m_distanceInches = Distances[i];
            return;
        }
    }

    // or well, fall back to defaults.
    */
}

unsigned int Shooter::CalcAngle()
{
    return ::CalcAngle(m_flag, m_distanceInches);
}

bool Shooter::GetAngleDown()
{
    return joystickGetDigital(7, JOY_DOWN);
}

bool IsUpperPosition()
{
    return joystickGetDigital(7, JOY_UP);
}

bool IsMiddlePosition()
{
    return joystickGetDigital(7, JOY_RIGHT);
}

// up: 100
// stop: 0
// down: -100
void Shooter::KeepMoving()
{
    int speed = 0;
    unsigned int current = analogRead(anglePotPort);
    int distance = current - m_angleToMove;
    int diff = distance - m_lastAngleDistance;

    // sometimes we get really wrong readings.
    if (diff > m_diffAdjusted + 12)
        diff = m_diffAdjusted + 12;
    else if (diff < m_diffAdjusted - 12)
        diff = m_diffAdjusted - 12;

    distance = diff + m_lastAngleDistance;
    unsigned int distanceAbs = abs(distance);

    m_diffAdjusted = (m_diffAdjusted + diff) / 2;

    m_count++;

    // Safety net - we want to stop after some time and let other steps in autonomous to play out.
    if (m_fMoving && (m_count >= 200 || (distanceAbs <= 10 && abs(m_diffAdjusted) <= 1)))
    {
        // printf("STOP: (%d) Speed: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, current - m_angleToMove, diff, m_diffAdjusted);
        StopMoving();
        return;
    }
    
    if (m_fMoving)
    {
        if (distance > 0) // going up
            speed = distance * 2 + m_diffAdjusted * (5 + abs(m_diffAdjusted)/5);
        else if (m_flag != Flag::Loading)
            speed = distance * 0.5 + m_diffAdjusted * 2;
        else
            speed = distance * 1.5 + m_diffAdjusted / 5;
        // printf("MOVE: (%d) Speed: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, current - m_angleToMove, diff, m_diffAdjusted);
    }
    else if (distanceAbs >= 15 && m_flag != Flag::Loading)
    {
        if (distance > 0) // going up
            speed = distance * 2 + m_diffAdjusted * 5;
        else
            speed = distance * 0.6 + m_diffAdjusted * 2;
        // printf("ADJUST: (%d) Speed: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, current - m_angleToMove, diff, m_diffAdjusted);
    }
    else
    {
        // m_count = 0;
        // printf("NON-MOVING: (%d) Speed: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, distancurrent - m_angleToMovece, diff, m_diffAdjusted);
    }

    // Go as fat as you can if loading
    const int angleMotorSpeed = (m_flag == Flag::Loading) ? 100 : 80;

    if (speed > angleMotorSpeed)
        speed = angleMotorSpeed;
    else if (speed < -angleMotorSpeed)
        speed = -angleMotorSpeed;

    motorSet(anglePort, speed);

    m_lastAngleDistance = distance;
}

void Shooter::StartMoving()
{
    m_fMoving = true;
    m_angleToMove = ConvertAngleToPotentiometer(CalcAngle());
    m_diffAdjusted = 0;
    m_lastAngleDistance = analogRead(anglePotPort) - m_angleToMove;
    m_count = 0;
}

void Shooter::StopMoving()
{
    motorSet(anglePort, 0);
    m_fMoving = false;
    m_count = 0;
}

void Shooter::Debug()
{
}

void Shooter::SetDistance(unsigned int distance)
{
    if (m_distanceInches == distance)
        return;
    m_distanceInches = distance;
    StartMoving();
}

void Shooter::SetFlag(Flag flag)
{
    if (m_flag == flag)
        return;
    m_flag = flag;
    StartMoving();
}

void Shooter::UpdateDistance()
{
    if (joystickGetDigital(8, JOY_LEFT))
        SetDistance(Distances[0]);
    else if (joystickGetDigital(8, JOY_UP))
        SetDistance(Distances[1]);
    else if (joystickGetDigital(8, JOY_RIGHT))
        SetDistance(Distances[2]);
    else if (joystickGetDigital(8, JOY_DOWN))
        SetDistance(Distances[3]);
}

void Shooter::Update()
{
    // Debug();

    unsigned int darkness = analogRead(lightSensor);
    bool ballIn = (darkness > lightSensorBallIn);
    bool ballOut = (darkness < lightSensorBallOut);

    /*
    if (!isAuto() && !m_Manual)
    {
        if (ballIn && m_flag == Flag::Loading)
            SetFlag(Flag::High);
        if (ballOut && m_flag != Flag::Loading)
            SetFlag(Flag::Loading);
    }
    */

    UpdateDistance();

    if (GetAngleDown())
    {
        if (m_flag != Flag::Loading)
        {
            if (ballIn)
                m_Manual = true;
            if (ballOut)
                m_Manual = false;
        }
        SetFlag(Flag::Loading);
    }
    else if (IsUpperPosition())
    {
        if (m_flag == Flag::Loading)
        {
            if (ballOut)
                m_Manual = true;
            if (ballIn)
                m_Manual = false;
        }
        SetFlag(Flag::High);
    }
    else if (IsMiddlePosition())
    {
        if (m_flag == Flag::Loading)
        {
            if (ballOut)
                m_Manual = true;
            if (ballIn)
                m_Manual = false;
        }
        SetFlag(Flag::Middle);
    }

    // Shooter
    if (joystickGetDigital(7, JOY_LEFT))
    {
        motorSet(shooterPort, shooterMotorSpeed);
    }
    else if (!isAuto())
    {
        motorSet(shooterPort, 0);
    }


    KeepMoving();
}