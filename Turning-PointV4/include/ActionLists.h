    Action* g_actionsFirstPos[] = {
        
        // Shoot the ball
        new ShooterAngle(Flag::Middle, 48),
        new ShootBall,
        new IntakeUp,
        
        // we are starting a bit at an angle to shoot, so need to fix this as we go in for low flag
        new Move(400, 60, AtonBlueRight ? 9 : -9),

        // turn closer to center, but then streighten to look direct at flag
        // This should allow us to look directly at flag, but also to be far away from border to turn around
        new Move(425, -60, AtonBlueRight ? 10 : -10),
        new Move(425, -60, AtonBlueRight ? -10 : 10),

        new IntakeStop,

        // Shooting the ball
        new ShooterAngle(Flag::High, 31),
        new Wait(50),
        new ShootBall,

        // Mving to center to flip cone
        new TurnToCenter(),
        new IntakeDown,
        new Move(800, 50),

        // Done for today
        new ActionNeverEnding
    };

    Action* g_ParkFromFirstPos[] = {
        // give some time for other robot to get out of the way
        // FYI: It takes about 7 seconds to park
        new Wait(600),
        new Move(400, 50),
        new TurnFromCenter(), // this is actually to center, it's in context of robot facing flags, but we are facing backfield
        new Move(1350, 85),
        new ActionNeverEnding
    };

    Action* g_ParkFromSecondPos[] = {
        // give some time for other robot to get out of the way
        // FYI: It takes about 7 seconds to park
        new Wait(600),
        new Move(400, 50),
        new TurnToCenter(),
        new Move(1350, 85),
        new ActionNeverEnding
    };

    Action* g_actionsNothing[] = {
        new ActionNeverEnding
    };
