#include "main.h"
#include "spinner.h"

bool Spinner::GetSpinner()
{
    return joystickGetDigital(1, 6, JOY_DOWN);
}

void Spinner::SetSpinnerMotor(float speed)
{
    motorSet(spinnerPort, speed);
}

void Spinner::ReadInputs()
{
    bool newSpinnerState = GetSpinner();
    if (newSpinnerState && !m_oldSpinnerState)
    {
        if (!m_on)
            Start();
        else
            Stop();
    }
    m_oldSpinnerState = newSpinnerState;
}

void Spinner::Start()
{
    m_on = true;
    int reading = analogRead(spinnerPotPort);
    m_target = (reading > 2000) ? 520 : 3560;
    m_lastError = m_target - reading;
    m_cyclesAfterStopped = 10000; // some big number
}

void Spinner::Stop()
{
    m_on = false;
    SetSpinnerMotor(0);
    printf("\n");
}

void Spinner::DebugRun()
{
    static int count = 0;
    count++;
    if (count > 200 && !(count % 500))
        Start();
}

void Spinner::Update()
{
   // DebugRun();
    ReadInputs();

    if (!m_on)
        return;

    m_cyclesAfterStopped--;
    if (m_cyclesAfterStopped == 0)
    {
        Stop();
        return;
    }

    int reading = analogRead(spinnerPotPort);
    int error = m_target - reading;
    int distance = abs(error);

    // at 350, we need to slow down a lot!
    int power = error;
    if (power > 500)
        power = 500;
    else if (power < -500)
        power = -500;

    int differential = error - m_lastError;
    int abs_differential = abs(differential);

    if (distance < 50)
    {
        if (abs_differential <= 10)
        {
            Stop();
            return;
        }
        // safety net- stop after 2 seconds, even if we can't get there...
        if (m_cyclesAfterStopped > 50)
            m_cyclesAfterStopped  = 50;
    }

    int newpower = power * 0.08 + differential * (0.1 + abs_differential * 0.005);
    // slow down rate of changes - use average of old & new power
    m_power = (m_power + newpower) / 2;

    printf("Count: %d Read: %d  Target: %d  Power: %d (%d) error: %d  Diff: %d\n",
            m_cyclesAfterStopped,
            analogRead(spinnerPotPort),
            m_target,
            m_power,
            power,
            error,
            differential);
 
    SetSpinnerMotor(-m_power);
    m_lastError = error;
}
