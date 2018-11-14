#include "lift.h"
#include "spinner.h"

// Bottom (grownd) position:
//    LL = 1220
//    RL = 2825
//    diff = 1605
// Flip height position:
//    LL = 1403 
//    RL = 2564
//    Diff = 1161
int Lift::CurrentHeight()
{
    return analogRead(rightLiftPotPort) - analogRead(leftLiftPotPort);
}

bool Lift::GetLiftUp()
{
    return joystickGetDigital(1, 5, JOY_UP);
}

bool Lift::GetLiftDown()
{
    return joystickGetDigital(1, 5, JOY_DOWN);
}

void Lift::SetLiftMotor(float speed)
{
    // slowly decrease speed when going up.
    if (speed == 0 && m_LiftSpeed < 0)
    {
        if (m_LiftSpeed < -100)
            m_LiftSpeed = -100;
        m_LiftSpeed = 0.97 * m_LiftSpeed + 0.03 * speed;
    }
    else
        m_LiftSpeed = speed;
    motorSet(liftPort, m_LiftSpeed);
}

void Lift::Update()
{
    bool liftUp = GetLiftUp();
    bool liftDown = GetLiftDown();
    bool liftUpAutomatic = Spinner::GetAutoSpinner();
    bool startrotation = liftUpAutomatic && !m_automaticKey;
    m_automaticKey = liftUpAutomatic;

    if (liftUp)
    {
        m_state = LiftState::Off;
        SetLiftMotor(-liftMotorSpeed);
        if (startrotation)
            spinner.Start();
    }
    else if (liftDown)
    {
        m_state = LiftState::Off;
        SetLiftMotor(liftMotorSpeed / 2);
    }
    else if (startrotation)
    {
        if (CurrentHeight() >= 1200)
        {  
            m_state = LiftState::Up;
            SetLiftMotor(-liftMotorSpeed/2);
        }
        // immidiattly start rotating - lift is vary fast!
        spinner.RotateAndDown();
    }
    else if (m_state == LiftState::Down)
    {
        if (CurrentHeight() > 1400)
        {
            SetLiftMotor(0);
            m_state = LiftState::Off;
        }
    }
    else if (m_state == LiftState::Up)
    {
        if (CurrentHeight() <= 1400)
        {
            SetLiftMotor(0);
            m_state = LiftState::Off;
        }
    }
    else
    {
        SetLiftMotor(0);
    }
}

// Spinner calls it when in automatic mode rotating cones
void Lift::MoveDown()
{
    int height = CurrentHeight();
    if (height < 1550 && (m_state == LiftState::Off || m_state == LiftState::Up))
    {
        m_state = LiftState::Down;
        SetLiftMotor(liftMotorSpeed/2);
    }
}
