#pragma once

#include "pros/adi.hpp"
#include "positionCore.h"

float GetGyroReading();

/*******************************************************************************
 *
 * Encoder class
 * 
 ******************************************************************************/
class Encoder
{
public:
  Encoder(std::uint8_t portTop, std::uint8_t portBottom, bool reverse);
  // Resets encoder for all usages:
  // All external accesses to this encoder will see current pos being reset
  void HardReset();
  // Returns cached position
  int GetPos() const;

private:
  pros::c::adi_encoder_t m_encoder;
};


/*******************************************************************************
 *
 * CMotor class
 * 
 ******************************************************************************/
class CMotor
{
public:
  CMotor(unsigned int port);
  // Resets motor for all usages:
  // All external accesses to this motor will see current pos being reset
  void HardReset();
  // Returns cached position
  int GetPos() const;

private:
  const unsigned int m_port;
};


// Main class
class PositionTracker : public PositionTrackerBase
{
private:
    CMotor m_motorLeftFront {leftFrontDrivePort};
    CMotor m_motorLeftBack {leftBackDrivePort};
    CMotor m_motorRightFront {rightFrontDrivePort};
    CMotor m_motorRightBack {rightBackDrivePort};
    Encoder m_leftEncoder {leftEncoderPortTop, leftEncoderPortBottom, true};
    Encoder m_rightEncoder {rightEncoderPortTop, rightEncoderPortBottom, true};

    bool m_flipX = false;
    float m_angleOffset = 0;

protected:
    void ReadSensors(Sensors& sensor) override;

public:
    PositionTracker();
    void ResetState();

    Position LatestPosition();
    void SetCoordinates(Position cord);
    void SetAngle(int degrees);
    float GetAngle();
    void FlipX(bool flip);
};

PositionTracker &GetTracker();
