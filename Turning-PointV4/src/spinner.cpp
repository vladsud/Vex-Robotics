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
        m_oldSpinnerState = newSpinnerState;
    }
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
    if (m_on)
    {
        m_on = false;
        SetSpinnerMotor(0);
        printf("\n");
        for (int i = 0; i < 5; i++)
        {
            delay(10);
            printf(" Count:      , Read: %d  Target: %d  (stopped)\n", analogRead(spinnerPotPort), m_target);
        }
    }
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
    // const int maxpower = 35;

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
    if (distance < 50 && m_cyclesAfterStopped > 10)
        m_cyclesAfterStopped = 10;

    // at 500, we need to slow down a lot!
    int power = error;
    if (power > 350)
        power = 350;
    else if (power < -350)
        power = -350;

    int differential = error - m_lastError;
    if (differential > 60)
        differential = 60;
    else if (differential < -60)
        differential = -60;
    error = differential + m_lastError;

    m_power = power * 0.1 + differential * 0.2;

    printf("Count: %d Read: %d  Target: %d  Power: %d (%d) LastErorr: %d  Diff: %d\n", m_cyclesAfterStopped, analogRead(spinnerPotPort), m_target, m_power, power, m_lastError, error - m_lastError);
    SetSpinnerMotor(-m_power);
    m_lastError = error;
}