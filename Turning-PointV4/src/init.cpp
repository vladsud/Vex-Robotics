#include "main.h"

Encoder g_leftDriveEncoder = nullptr;
Encoder g_rightDriveEncoder = nullptr;
Gyro g_gyro = nullptr;

void initialize()
{
  g_leftDriveEncoder = encoderInit(leftDriveEncoderTopPort, leftDriveEncoderBotPort, true);
  g_rightDriveEncoder = encoderInit(rightDriveEncoderTopPort, rightDriveEncoderBotPort, true);
  g_gyro = gyroInit(gyroPort, 0/*multiplier, 0-default*/);

  ReportStatus("Init: Encoders: %p, %p, Gyro: %p\n", g_leftDriveEncoder, g_rightDriveEncoder, g_gyro);

  delay(2000);
}
