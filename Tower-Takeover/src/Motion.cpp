#include "motion.h"
#include "cycle.h"
const Model moveModel = {
    45,  /*initialPower */
    0,   /* basePower */
    1.8, /* speedCoeff */
    4.0,  /* diffCoeff */
    { 40, 150, 300, 4800, UINT_MAX},
    { 10, 20,  30,  240, 240},
};

const Model turnModel = {
    45,  /*initialPower */
    0,   /* basePower */
    0.4, /* speedCoeff */
    0.5, /* diffCoeff */
    { 50,  500, 6000, UINT_MAX},
    {  30, 50,   200, 200},
};

int Sign(int value)
{
   if (value < 0)
      return -1;
   if (value > 0)
      return 1;
   return 0;
}

// unsigned int points[] {30, 50, 1600, UINT_MAX};
// unsigned int speeds[] {0,  60, 4000, 4000};
unsigned int SpeedFromDistances(unsigned int distance, const unsigned int* points, const unsigned int* speeds)
{
    unsigned int lowPoint = 0;
    unsigned int lowSpeed = 0;
    while (true)
    {
        unsigned int highPoint = *points;
        unsigned int highSpeed = *speeds;
        Assert(lowPoint < highPoint);
        Assert(lowSpeed <= highSpeed);
        if (distance <= highPoint)
            return lowSpeed + (highSpeed - lowSpeed) * (distance - lowPoint) / (highPoint - lowPoint);
        points++;
        speeds++;
        lowPoint = highPoint;
        lowSpeed = highSpeed;
    }
}

int SpeedFromDistances(int distance, const unsigned int* points, const unsigned int* speeds)
{
    return SpeedFromDistances((unsigned int)abs(distance), points, speeds) * Sign(distance);
}

Motion::Motion(const Model& model, unsigned int speedLimit)
    : m_speedLimit(speedLimit),
    m_model(model)
{
}

int Motion::SpeedFromDistance(int error)
{
    return SpeedFromDistances(error, m_model.points, m_model.speeds);
}


bool Motion::ShouldStop()
{
    int error = GetError();
    int sign = Sign(error);
    int actualSpeed = m_lastError - error;
    m_lastError = error;

    if (m_firstRun)
    {
        m_firstRun = false;
        m_initialError = error;
        actualSpeed = 0;
        m_power = sign * (int)m_model.initialPower;
    }

    int idealNow = SpeedFromDistance(error);
    int idealSpeed = (idealNow + SpeedFromDistance(error - 2 * actualSpeed)) / 2;

    if (abs(idealNow) > m_speedLimit)
        idealNow = m_speedLimit * Sign(idealSpeed);
    if (abs(idealSpeed) > m_speedLimit)
        idealSpeed = m_speedLimit * Sign(idealSpeed);

    int diff = idealSpeed - actualSpeed;

    // Define "target" as first point in array of points
    unsigned int target = m_model.points[0];

    int power = 0;
    int result = false;

    // Are we within target? Or way past target?
    if (abs(error) <= target || Sign(m_initialError) * error < 0)
    {
        // Estimate where we would stop with no power.
        // Assumes 1.5 drop in speed on each cycle without zero motor power
        m_stopPoint = error - Sign(actualSpeed) * actualSpeed * actualSpeed / 2;
        // If within half of range - stop. Or if the speed is below one at target
        if (abs(m_stopPoint) <= target / 2 || abs(actualSpeed) <= abs(SpeedFromDistance(target) / 2))
        {
            result = true;
            /*
            AssertSz(abs(error) <= target && abs(m_stopPoint) <= target / 2,
                    "Motion Overshooted: err = %d, estimated stopping point = %d, speed = %d\n",
                    error,
                    m_stopPoint,
                    actualSpeed);
                    */
        }
    }

    if (!result)
    {
        // Power calculation.
        power = sign * m_model.basePower + idealSpeed * m_model.speedCoeff + diff * m_model.diffCoeff;

        // Formula used above for calculating power are approximations.
        // As result, power will fluctuated up and down.
        // This allows us to find more accurate power setting to maintain required speed
        int maxStep = 20;
        if (power > m_power + maxStep)
            power = m_power + maxStep;
        else if (power < m_power - maxStep)
            power = m_power - maxStep;
        m_power = power;

        int powerLimit = MotorMaxSpeed;
        if (m_power > powerLimit)
            m_power = powerLimit;
        else if (m_power < -powerLimit)
            m_power = -powerLimit;
    }

    ReportStatus(Log::Motion, "%s err =%5d, angle = %f, speed =%3d, ideal =%3d, power =%4d / %d\n",
        result ? "DONE:" : "",
        error, GetGyroReading(), actualSpeed, idealNow, power, m_power);

    SetMotorPower(m_power);
    return result;
}
