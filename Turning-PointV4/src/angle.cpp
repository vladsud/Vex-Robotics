#include "angle.h"

// Angle potentiometer:
const int anglePotentiometerLow = 1790;
const int anglePotentiometerHigh = 1060;

constexpr int Distances[]    {   36 ,  48,  60,   72, 4*24+12};
constexpr int AnglesHigh[]   {1150, 1300, 1350, 1430, 1600};
constexpr int AnglesMedium[] {1250, 1400, 1450, 1480, 1650};

#define CountOf(a) (sizeof(a)/sizeof(a[0]))
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

static_assert(CalcAngle(Flag::Loading, Distances[2]) == anglePotentiometerLow);

static_assert(CalcAngle(Flag::High, Distances[0]-5) == AnglesHigh[0]);
static_assert(CalcAngle(Flag::High, Distances[0]) == AnglesHigh[0]);
static_assert(CalcAngle(Flag::High, Distances[2]) == AnglesHigh[2]);
static_assert(CalcAngle(Flag::High, Distances[LastDistanceCount]) == AnglesHigh[LastDistanceCount]);
static_assert(CalcAngle(Flag::High, Distances[LastDistanceCount]+100) == AnglesHigh[LastDistanceCount]);
static_assert(CalcAngle(Flag::High, (Distances[2] + Distances[3]) / 2) == (AnglesHigh[2] + AnglesHigh[3]) / 2);
static_assert(CalcAngle(Flag::High, (Distances[3]*3 + Distances[4]) / 4) == (AnglesHigh[3]*3 + AnglesHigh[4]) / 4);

static_assert(CalcAngle(Flag::Middle, Distances[0]-5) == AnglesMedium[0]);
static_assert(CalcAngle(Flag::Middle, Distances[0]) == AnglesMedium[0]);
static_assert(CalcAngle(Flag::Middle, Distances[2]) == AnglesMedium[2]);
static_assert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]) == AnglesMedium[LastDistanceCount]);
static_assert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]+100) == AnglesMedium[LastDistanceCount]);
static_assert(CalcAngle(Flag::Middle, (Distances[2] + Distances[3]) / 2) == (AnglesMedium[2] + AnglesMedium[3]) / 2);
static_assert(CalcAngle(Flag::Middle, (Distances[3]*3 + Distances[4]) / 4) == (AnglesMedium[3]*3 + AnglesMedium[4]) / 4);

// Need to figure out initial position
Angle::Angle()
{
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
}

int Angle::CalcAngle()
{
    return ::CalcAngle(m_flag, m_distanceInches);
}

bool Angle::GetAngleDown()
{
    return joystickGetDigital(1, 7, JOY_DOWN);
}

bool IsUpperPosition()
{
    return joystickGetDigital(1, 7, JOY_UP);
}

bool IsMiddlePosition()
{
    return joystickGetDigital(1, 7, JOY_RIGHT);
}

// up: 100
// stop: 0
// down: -100
void Angle::KeepMoving()
{
    int speed = 0;
    int current = analogRead(anglePotPort);
    int distance = current - m_angleToMove;
    int distanceAbs = abs(distance);
    int diff = distance - m_lastAngleDistance;

    m_diffAdjusted = (m_diffAdjusted + diff) / 2;

    m_count++;

    // overshooting
    if (m_count >= 150 || (distanceAbs <= 20 && abs(diff) <= 2) || (distanceAbs <= 15 && abs(diff) <= 3))
    {
        StopMoving();
        return;
    }
    else if (distance > 0) // up
        speed = distance *0.7 + m_diffAdjusted * 4.5;
    else
        speed = distance *0.5 + m_diffAdjusted * 2.8;

    if (speed > angleMotorSpeed)
        speed = angleMotorSpeed;
    else if (speed < -angleMotorSpeed)
        speed = -angleMotorSpeed;

    printf("(%d) Speed: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, distance, diff, (int)m_diffAdjusted);

    m_lastAngleDistance = distance;
    
    motorSet(anglePort, speed);
}

void Angle::StartMoving(int angle)
{
    m_fMoving = true;
    m_angleToMove = angle;
    m_diffAdjusted = 0;
    m_lastAngleDistance = analogRead(anglePotPort) - m_angleToMove;
    m_count = 0;
}

void Angle::StopMoving()
{
   motorSet(anglePort, 0);
    m_fMoving = false;
}

void Angle::Debug()
{
    SetFlag(Flag::Loading);
    for (int i = 0; i < 100; i++)
    {
        KeepMoving();
        delay(10);
    }

    //StartMoving(CalcAngle());

    printf("\n1\n");

    SetDistance(48);
    SetFlag(Flag::Middle);
    for (int i = 0; i < 200; i++)
    {
        KeepMoving();
        delay(10);
    }

    printf("\n2\n");
    SetFlag(Flag::High);
    for (int i = 0; i < 200; i++)
    {
        KeepMoving();
        delay(10);
    }

    printf("\n3\n");
    SetFlag(Flag::Middle);
    for (int i = 0; i < 200; i++)
    {
        KeepMoving();
        delay(10);
    }

    printf("\n4\n");
    SetFlag(Flag::High);
    for (int i = 0; i < 200; i++)
    {
        KeepMoving();
        delay(10);
    }

    printf("\n5\n");
    SetFlag(Flag::Middle);
    for (int i = 0; i < 200; i++)
    {
        KeepMoving();
        delay(10);
    }
}

void Angle::SetDistance(int distance)
{
    if (m_distanceInches == distance)
        return;
    m_distanceInches = distance;
    StartMoving(CalcAngle());
}

void Angle::SetFlag(Flag flag)
{
    if (m_flag == flag)
        return;
    m_flag = flag;
    StartMoving(CalcAngle());
}

void Angle::UpdateDistance()
{
    if (joystickGetDigital(1, 8, JOY_LEFT))
        SetDistance(36);
    else if (joystickGetDigital(1, 8, JOY_UP))
        SetDistance(48);
    else if (joystickGetDigital(1, 8, JOY_RIGHT))
        SetDistance(60);
    else if (joystickGetDigital(1, 8, JOY_DOWN))
        SetDistance(72);
}

void Angle::Update()
{
    // Debug();

    UpdateDistance();

    if (GetAngleDown())
    {
        SetFlag(Flag::Loading);
    }
    else if (IsUpperPosition())
    {
        SetFlag(Flag::High);
    }
    else if (IsMiddlePosition())
    {
        SetFlag(Flag::Middle);
    }

    if (m_fMoving)
        KeepMoving();
}