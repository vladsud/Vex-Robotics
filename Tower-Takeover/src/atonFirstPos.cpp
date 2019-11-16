#include "aton.h"
#include "atonFirstPos.h"
#include "pros/motors.h"

using namespace pros;
using namespace pros::c;
// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

struct Initialization : public Action
{
    public:
    Initialization()
    {
        printf("Start Initialization\n\n");
        GetMain().sm.SetState(State::InitializationState);
    }
    bool ShouldStop() override
    {
        //printf("Lift: %d cubetray: %d\n", GetMain().lift.m_anglePot.get_value(), GetMain().cubetray.m_anglePot.get_value());
        if (GetMain().lift.m_anglePot.get_value() < 1400 && GetMain().cubetray.m_anglePot.get_value() < 2000)
        {
            return true;
        }
        return false;
    }

    void Stop()  override
    {
        printf("Stopped Initialization\n");
        motor_move(liftMotorPort, 0);
        motor_move(cubetrayPort, 0);
    }
};

struct Reset : public Action
{
    public:
    Reset()
    {
        GetMain().sm.SetState(State::Rest);
        printf("Started Reset \n\n");
    }
    bool ShouldStop()  override
    {
        if (GetMain().lift.m_anglePot.get_value() >= 2200 && GetMain().cubetray.m_anglePot.get_value() >= 2850)
        {
            return true;
        }
        return false;
    }

    void Stop()  override
    {
        motor_move(liftMotorPort, 0);
        motor_move(cubetrayPort, 0);
        printf("Stopped Reset \n\n");
    }
};

struct Unload : public Action
{
    public:
    Unload()
    {
        GetMain().sm.SetState(State::TrayOut);
    }
    bool ShouldStop() override
    {
        if (GetMain().cubetray.m_anglePot.get_value() > 2000)
        {
            return true;
        }
        return false;
    }

    void Stop() override
    {
        motor_move(liftMotorPort, 0);
        motor_move(cubetrayPort, 0);
    }
};

void RunAtonFirstPos()
{
    auto &main = GetMain();
    auto timeBegin = main.GetTime();

    MoveExact(2000, 0);

    Do(Initialization());
    Do(Reset());

    GetMain().sm.intakeOverride = true;
    motor_move(intakeLeftPort, GetMain().intake.intake_normal_speed);
    motor_move(intakeRightPort, -GetMain().intake.intake_normal_speed);

    MoveExact(10000, 0);
    MoveExact(7000, -155);

    Do(Unload());
    MoveExact(-2000, -155);
    GetMain().sm.intakeOverride = false;
}
