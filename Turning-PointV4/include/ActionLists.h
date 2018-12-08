Action* g_actionsFirstPos[] = {
    
    // Shoot the ball
    new ShooterAngle(g_lcd.AtonShootHighFlag, 48, false /*checkPresenceOfBall*/),
    new ShootBall,
    new IntakeUp,
    
    // we are starting a bit at an angle to shoot, so need to fix this as we go in for low flag
    new Move(400, 60, 0),
    new Move(1000, 60, g_lcd.AtonBlueRight ? 5 : -5),
    new GyroOff(),
    new Move(600, 85, 0),
    new Wait(100),
    new GyroOn(),

    // turn closer to center, but then streighten to look direct at flag
    // This should allow us to look directly at flag, but also to be far away from border to turn around
  //   new Move(600, -60, 0),
    new Move(1500, -60, g_lcd.AtonBlueRight ? -5 : 5),
    // new Turn(g_lcd.AtonBlueRight ? -10 : 10),
    // new Move(600, -60, 0),
    //new Move(600, -60, g_lcd.AtonBlueRight ? 5 : -5),
    // new Turn(g_lcd.AtonBlueRight ? 10 : -10),

    new IntakeStop,

    // Shooting the ball
    new ShooterAngle(!g_lcd.AtonShootHighFlag, 31, true /*checkPresenceOfBall*/),
    new Wait(50),
    new ShootBall
};

Action* g_knockConeFirstPos[] = {
    // Moving to center to flip cone
    //new TurnToCenter(),
    //new IntakeDown,
    //new Move(800, 50),
};

Action* g_ParkFromFirstPos[] = {
    // give some time for other robot to get out of the way
    // FYI: It takes about 7 seconds to park
    new Wait(500),
    new Move(400, 50),
    new TurnFromCenter(), // this is actually to center, it's in context of robot facing flags, but we are facing backfield
    new Move(1350, 85),
};

Action* g_ShootFromSecondPos[] = {
    new ShooterAngle(g_lcd.AtonShootHighFlag, 96, false /*checkPresenceOfBall*/),
    // give some time for other robot to get out of the way
    // FYI: It takes about 7 seconds to park
    new Wait(400),
    new ShootBall,
};

Action* g_WaitInsteadOfShot[] = {
    new Wait(450)
};

// FYI: It takes about 6 seconds to park
Action* g_ParkFromSecondPos[] = {
    new Move(400, 50),
    new TurnToCenter(),
    new Move(1350, 85),
};

// NYI
Action* g_knockConeSecondPos[] = {};

unsigned int TestDistance = 24;
Action*g_testAngles[] = {
    // new ShooterAngle(Flag::Loading, TestDistance, false /*checkPresenceOfBall*/),
    // new Wait(50),
    new ShooterAngle(false, TestDistance, false /*checkPresenceOfBall*/),
    new Wait(50),
    new ShooterAngle(true, TestDistance, false /*checkPresenceOfBall*/),
    new Wait(50),
    new ShooterAngle(false, TestDistance, false /*checkPresenceOfBall*/),
    new Wait(50),
};


Action*g_testDrive[] = {
    new Turn(45),
    new Wait(50),
    new Turn(45),
};
