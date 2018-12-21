#include "position.h"
#include <cmath>

PositionTracker::PositionTracker()
{
    long time = millis();
    for (int i = 0; i < SamplesToTrack; i++)
    {
        m_time[i] = time + (SamplesToTrack-i)*PositionTrackingRefreshRate;

        m_sensor.leftEncoder[i] = 0;
        m_sensor.rightEncoder[i] = 0;
        m_sensor.sideEncoder[i] = 0;
        m_sensor.leftMotorPower[i] = 0;
        m_sensor.rightMotorPower[i] = 0;
        m_sensor.gyro[i] = 0;
        m_sensor.shooterAngle[i] = 0;
        m_sensor.shooterAnglePower[i] = 0;

        m_position.leftSpeed[i] = 0;
        m_position.rightSpeed[i] = 0;
        m_position.sideSpeed[i] = 0;
        m_position.X[i] = 0;
        m_position.Y[i] = 0;
        m_position.gyroSpeed[i] = 0;
        m_position.shooterAngle[i] = 0;
    }

    m_time[0] = time;
}


// Initial smoothing gives us a bit more precision accuracy, as we convert integers (with +/-1 error) to floats (+/-0.33 error)
// Note this process is stable (dispite being partially recursive), as we always asomptotically reach stable value.
template <typename T1>
T1 PositionTracker::SmoothSeries(T1* dataIn)
{
    T1 value = 0;
    for (int i = 0; i < 2 * initialSmoothingRange + 1; i++)
        value += Value(dataIn, m_currentIndex - i);
    
    value /= 2 * initialSmoothingRange + 1;
    Value(dataIn, m_currentIndex - initialSmoothingRange) = value;
    return value;
}

template <typename T1, typename T2>
void PositionTracker::SmoothSeries(T1* dataIn, T2* dataSpeedOut)
{
    int middleIndex = (m_currentIndex + SamplesToTrack - finalSmoothingRange - initialSmoothingRange) % SamplesToTrack;

    // First we smothen last previous point
    T2 x2 = SmoothSeries(dataIn);
    T2 x1 = dataIn[middleIndex];
    T2 x0 = Value(dataIn, m_currentIndex - 2 * finalSmoothingRange - initialSmoothingRange);

    // Now let's figure out velocity!
    // we assume that the rate of change has fixed acceleration.
    // We calculate speed & acceleration using 3 points further out from each other.
    // Note that we can calculate middle one using past & present data, which is likely more accuratevand more stable.
    // But we also can calculate current speed (more axxurately then just substricing nareast values) and that
    // can be used as temporary metric helping us estimate our position.
    //
    // Note this process is stable, as integrating speeds over time would produce final value.
    //
    // a = (x2 + x0 - 2*x1) /(t*t)
    // v0 = (-3*x0 + 4*x1 - x2) / (2*t)
    // V1 = x1 - x0
    // V2 = (x2 - x0) / 2
    // acceleration = x0 - 2 * x1 + x2;
    // SpeedInitial = (-3 * x0 + 4 * x1 - x2) / 2;
    // valueMid = x0 + SpeedInitial + acceleration;
    dataSpeedOut[middleIndex] = (x2 - x0) / (2*finalSmoothingRange);
    Value(dataSpeedOut, m_currentIndex-initialSmoothingRange) = (x0 - 4*x1 + 3 * x2) / (2*finalSmoothingRange);
}

bool PositionTracker::RecalcPosition(int index)
{
    Assert(index == ((index + SamplesToTrack) % SamplesToTrack));

    double arcMoveForward = (m_position.leftSpeed[index] + m_position.rightSpeed[index]) / 2;
    double sideMove = m_position.sideSpeed[index];
    int indexPrev = (index + SamplesToTrack - 1) % SamplesToTrack;

    double X = m_position.X[index];
    double Y = m_position.Y[index];

    if (arcMoveForward == 0 && sideMove == 0)
    {
        m_position.X[index] = m_position.X[indexPrev];
        m_position.Y[index] = m_position.Y[indexPrev];
    }
    else
    {
        // Positive is counterclockwise, same as gyro
        // These 2 was of calculating angle should produce similar results, but see notes on resolution above...
        // double angle = (rightEncoder - leftEncoder) * one_by_wheelDistance;
        double angle = m_sensor.gyro[index] * GyroToRadiants;

        // Condition should be: angleDiffHalf != 0
        // But in practice,
        //     sin(pi/180)/(pi/180) = 0.99995
        //     sin(0.4*pi/180)/(0.4*pi/180) = 0.99999
        double gyroSpeed = m_position.gyroSpeed[index];
        double angleDiffHalf = gyroSpeed * GyroToRadiants / 2;
        if (abs((int)gyroSpeed) >= GyroWrapper::Multiplier / 8)
        {
            double coeff = sin (angleDiffHalf) / angleDiffHalf;
            arcMoveForward *= coeff;
            sideMove *= coeff;
        }
        double angleHalf = angle - angleDiffHalf;
        double sinA = sin(angleHalf);
        double cosA = cos(angleHalf);
        m_position.X[index] = m_position.X[indexPrev] - sinA * arcMoveForward;
        m_position.Y[index] = m_position.Y[indexPrev] + cosA * arcMoveForward;
        if (sideMove != 0)
        {
            m_position.X[index] += - cosA * sideMove;
            m_position.Y[index] += - sinA * sideMove;
        }
    }

    return X != m_position.X[index] || Y != m_position.Y[index];
}


void PositionTracker::SmoothSensor()
{
    SmoothSeries(m_sensor.leftEncoder, m_position.leftSpeed);
    SmoothSeries(m_sensor.rightEncoder, m_position.rightSpeed);
    SmoothSeries(m_sensor.sideEncoder, m_position.sideSpeed);
    SmoothSeries(m_sensor.gyro, m_position.gyroSpeed);

    // SmoothSeries(m_sensor.leftMotorPower);
    // SmoothSeries(m_sensor.rightMotorPOwer);
    // SmoothSeries(m_sensor.shooterAngle, m_position.shooterAngle);
    // SmoothSeries(m_sensor.shooterAnglePower);
}

// Based on http://thepilons.ca/wp-content/uploads/2018/10/Tracking.pdf
// We are missing here third wheel, so we can't account for bumps form the side.
//
// ***         ***
// ***  TURNS  ***
// ***         ***
//      Full in-place turn == 360*3.547 clicks on each wheel.
//      Or roughly 0.14 degrees per one click.
//      Gyro, on another hand, 0.004 degree resolution (or 35.5 times better than wheel resolution). 
//      Max full turn in one second == 360 degree/second, measurements per 10ms:
//          - 25.5 click difference between wheels
//          - Or 256*3.6 = 921.6 clicks on gyro.
//
// ***         ***
// ***  MOVING ***
// ***         ***
//      One tick is roughly 0.9 mm
//      Assuming 24" / second speed:
//              670 clicks / second
//              6.7 clicks per 10ms
//      Assuming extreame case (which is not possible)
//          1 click / millisecond = 35" / second
//      Tthat said, we exceeed that rate when turning, as robot is not really moving (less energy spent per click)
//
void PositionTracker::Update()
{
    int leftEncoder = encoderGet(g_leftDriveEncoder);
    int rightEncoder = encoderGet(g_rightDriveEncoder);
    int sideEncoder = encoderGet(g_sideEncoder);
    m_gyro = GyroWrapper::Get();

    int i = (m_currentIndex + 1) % SamplesToTrack;
    m_currentIndex = i;
    m_time[i] = millis();
    m_sensor.leftEncoder[i] = leftEncoder;
    m_sensor.rightEncoder[i] =  rightEncoder;
    m_sensor.sideEncoder[i] = sideEncoder;
    m_sensor.leftMotorPower[i] = 0;
    m_sensor.rightMotorPower[i] = 0;
    m_sensor.gyro[i] = m_gyro;
    m_sensor.shooterAngle[i] = 0;
    m_sensor.shooterAnglePower[i] = 0;

    if (PrintDiagnostics(Diagnostics::Position))
    {
        printf("Tracking: %d %d %d %d\n", leftEncoder, rightEncoder, sideEncoder, m_gyro);
    }

    SmoothSensor();

    int MiddlePos =(m_currentIndex + SamplesToTrack - initialSmoothingRange - finalSmoothingRange) % SamplesToTrack;

    // As part of smoothing, we've updated velocities for middle point.
    // Now we can recalculate all positions from that point to current point.
    // But stop recalculation if nothing changed - that saved time.
    while (MiddlePos != m_currentIndex && RecalcPosition(MiddlePos))
    {
        MiddlePos = (MiddlePos + 1) % SamplesToTrack;
    };

    // Alwasy recalc latest point.
    (void)RecalcPosition(m_currentIndex);

    m_count++;
    const int halfCycle = 1000;
    int udpateCycle = m_count % (2*halfCycle);
    if (udpateCycle == 0)
        lcdPrint(uart1, 1, "XYA %d %d %d",
            int(m_position.X[m_currentIndex]), int(m_position.Y[m_currentIndex]),
            int(m_sensor.gyro[m_currentIndex] / GyroWrapper::Multiplier));
    if (udpateCycle == 1)
        lcdPrint(uart1, 2, "LRA: %d %d %d",
            encoderGet(g_leftDriveEncoder), encoderGet(g_rightDriveEncoder), int(m_gyro / GyroWrapper::Multiplier));
}
