#include "main.h"
#include "cycle.h"

Encoder g_leftDriveEncoder = nullptr;
Encoder g_rightDriveEncoder = nullptr;
Gyro g_gyro = nullptr;

void initialize()
{
  g_leftDriveEncoder = encoderInit(leftDriveEncoderTopPort, leftDriveEncoderBotPort, true);
  g_rightDriveEncoder = encoderInit(rightDriveEncoderTopPort, rightDriveEncoderBotPort, true);
  g_gyro = gyroInit(gyroPort, 0/*multiplier, 0-default*/);

  ReportStatus("Init: Encoders: %p, %p, Gyro: %p\n", g_leftDriveEncoder, g_rightDriveEncoder, g_gyro);

  setTeamName("Fluxion");

  g_lcd.Init();

  // Don't do it - this code looks like runs both in autonomous & manul modes. 
  // sit still for Gyro to calibrate
  // delay(2000);
}
