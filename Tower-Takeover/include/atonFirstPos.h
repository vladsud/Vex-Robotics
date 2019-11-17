#pragma once
#include "main.h"
#include "pros/motors.h"

using namespace pros;
using namespace pros::c;

extern const int distanceToCap;


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
        if (GetMain().lift.m_anglePot.get_value() < 1400 && GetMain().cubetray.m_anglePot.get_value() < 2500)
        {
            return true;
        }
        return false;
    }

    void Stop()  override
    {
        printf("Stopped Initialization\n");
        motor_move( liftMotorPort, 0);
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
        if (GetMain().cubetray.m_anglePot.get_value() < 1200)
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
