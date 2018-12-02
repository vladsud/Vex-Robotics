#include "angle.h"

// distance based on front of robot
constexpr int Distances[]    {   24 ,  30,  48,   54, 4*24+12};
constexpr int AnglesHigh[]   {1220, 1380,  1510, 1530, 1600};
constexpr int AnglesMedium[] {1440, 1500, 1670, 1671, 1680};

constexpr int LastDistanceCount = CountOf(Distances)-1;

static_assert(CountOf(Distances) == CountOf(AnglesHigh));
static_assert(CountOf(Distances) == CountOf(AnglesMedium));

static_assert(Distances[0] < Distances[1]);
static_assert(Distances[1] < Distances[2]);
static_assert(Distances[2] < Distances[3]);
static_assert(Distances[3] < Distances[4]);

static_assert(AnglesHigh[0] < AnglesHigh[1]);
static_assert(AnglesHigh[1] < AnglesHigh[2]);
static_assert(AnglesHigh[2] < AnglesHigh[3]);
static_assert(AnglesHigh[3] < AnglesHigh[4]);

static_assert(AnglesMedium[0] < AnglesMedium[1]);
static_assert(AnglesMedium[1] < AnglesMedium[2]);
static_assert(AnglesMedium[2] < AnglesMedium[3]);
static_assert(AnglesMedium[3] < AnglesMedium[4]);

static_assert(AnglesMedium[0] > AnglesHigh[0]);
static_assert(AnglesMedium[1] > AnglesHigh[1]);
static_assert(AnglesMedium[2] > AnglesHigh[2]);
static_assert(AnglesMedium[3] > AnglesHigh[3]);
static_assert(AnglesMedium[4] > AnglesHigh[4]);


constexpr int CalcAngle(Flag flag, int distanceInches)
{
    if (flag == Flag::Loading)
        return anglePotentiometerLow;

    const int* angles = (flag == Flag::High) ? AnglesHigh : AnglesMedium;
    unsigned int count = CountOf(Distances);

    if (Distances[0] >= distanceInches)
        return angles[0];

    for (unsigned int i = 0; i < count; i++)
    {
        if (Distances[i] >= distanceInches)
        {
            return angles[i-1] + (distanceInches - Distances[i-1]) * (angles[i] - angles[i-1]) / (Distances[i] - Distances[i-1]);
        }
    }

    return angles[count-1];
}

constexpr bool AlmostSameAngle(int angle1, int angle2)
{
    return abs(angle1-angle2) <= 2;
}

static_assert(CalcAngle(Flag::Loading, Distances[2]) == anglePotentiometerLow);

static_assert(CalcAngle(Flag::High, Distances[0]-5) == AnglesHigh[0]);
static_assert(CalcAngle(Flag::High, Distances[0]) == AnglesHigh[0]);
static_assert(CalcAngle(Flag::High, Distances[2]) == AnglesHigh[2]);
static_assert(CalcAngle(Flag::High, Distances[LastDistanceCount]) == AnglesHigh[LastDistanceCount]);
static_assert(CalcAngle(Flag::High, Distances[LastDistanceCount]+100) == AnglesHigh[LastDistanceCount]);
static_assert(CalcAngle(Flag::High, (Distances[2] + Distances[3]) / 2) == (AnglesHigh[2] + AnglesHigh[3]) / 2);
static_assert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[3]*3 + Distances[4]) / 4),(AnglesHigh[3]*3 + AnglesHigh[4]) / 4));

static_assert(CalcAngle(Flag::Middle, Distances[0]-5) == AnglesMedium[0]);
static_assert(CalcAngle(Flag::Middle, Distances[0]) == AnglesMedium[0]);
static_assert(CalcAngle(Flag::Middle, Distances[2]) == AnglesMedium[2]);
static_assert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]) == AnglesMedium[LastDistanceCount]);
static_assert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]+100) == AnglesMedium[LastDistanceCount]);
static_assert(CalcAngle(Flag::Middle, (Distances[2] + Distances[3]) / 2) == (AnglesMedium[2] + AnglesMedium[3]) / 2);
static_assert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[3]*3 + Distances[4]) / 4), (AnglesMedium[3]*3 + AnglesMedium[4]) / 4));

// Need to figure out initial position
Shooter::Shooter()
{
    StartMoving(CalcAngle());

    /*
    int precision = 50;
    int angle = analogRead(anglePotPort);
    
    if (abs(angle - anglePotentiometerLow) < precision)
    {
        m_flag = Flag::Loading;
        return;
    }

    for (unsigned int i = 0; i < CountOf(Distances); i++)
    {
        if (abs(angle - AnglesHigh[i]) < precision)
        {
            m_flag = Flag::High;
            m_distanceInches = Distances[i];
            return;
        }
    }

    for (unsigned int i = 0; i < CountOf(Distances); i++)
    {
        if (abs(angle - AnglesMedium[i]) < precision)
        {
            m_flag = Flag::Middle;
            m_distanceInches = Distances[i];
            return;
        }
    }

    // or well, fall back to defaults.
    */
}

int Shooter::CalcAngle()
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
    int current = analogRead(anglePotPort);
    int distance = current - m_angleToMove;
    int distanceAbs = abs(distance);
    int diff = distance - m_lastAngleDistance;

    m_diffAdjusted = (m_diffAdjusted + diff) / 2;

    m_count++;

    // Safety net - we want to stop after some time and let other steps in autonomous to play out.
    if (m_count >= 200 || (distanceAbs <= 10 && abs(m_diffAdjusted) <= 2))
    {
        // printf("STOP: (%d) Speed: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, distance, diff, (int)m_diffAdjusted);
        StopMoving();
        return;
    }
    
    if (m_fMoving)
    {
        if (distance > 0) // going up
            speed = distance * 0.6 + m_diffAdjusted * 6;
        else if (m_flag == Flag::Loading)
            speed = distance * 1.4 + m_diffAdjusted * 3;
        else
            speed = -5 + distance *0.5 + m_diffAdjusted * 2; // we want to overshoot to compensate against physical limitations
        //printf("(%d) Speed: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, distance, diff, (int)m_diffAdjusted);
    }
    // second of quite time after stop - let the system rest.
    else if (m_count <= 100 && distanceAbs >= 15)
    {
        speed = distance * 1.2 + m_diffAdjusted * 5.5; // 0.7 % 4.5

        printf("(%d) Speed: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, distance, diff, (int)m_diffAdjusted);
    }

    const int angleMotorSpeed = 40;

    if (speed > angleMotorSpeed)
        speed = angleMotorSpeed;
    else if (speed < -angleMotorSpeed)
        speed = -angleMotorSpeed;

    motorSet(anglePort, speed);

    m_lastAngleDistance = distance;
}

void Shooter::StartMoving(int angle)
{
    m_fMoving = true;
    m_angleToMove = angle;
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
    for (int i = 0; i < 100; i++)
    {
        KeepMoving();
        delay(10);
    }

    print("\n1111\n\n");

    SetFlag(Flag::Middle);
    SetDistance(Distances[0]);
    for (int i = 0; i < 100; i++)
    {
        KeepMoving();
        delay(10);
    }

    print("\n2222\n\n");

    SetFlag(Flag::Middle);
    SetDistance(Distances[1]);
    for (int i = 0; i < 100; i++)
    {
        KeepMoving();
        delay(10);
    }

    print("\n3333\n\n");

    SetFlag(Flag::Middle);
    SetDistance(Distances[0]);
    for (int i = 0; i < 100; i++)
    {
        KeepMoving();
        delay(10);
    }

    print("\n4444\n\n");
}

void Shooter::SetDistance(int distance)
{
    if (m_distanceInches == distance)
        return;
    m_distanceInches = distance;
    StartMoving(CalcAngle());
}

void Shooter::SetFlag(Flag flag)
{
    if (m_flag == flag)
        return;
    m_flag = flag;
    StartMoving(CalcAngle());
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

    int darkness = analogRead(lightSensor);
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