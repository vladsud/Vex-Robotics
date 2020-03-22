/*
 * PROS Analog Gyro Library for VEX Yaw-Rate 1000dps LY3100ALH gyro
 *
 * Copyright (c) 2011-2016, Purdue University ACM SIGBots.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */
#include "main.h"
#include "gyro.h"
#include "drive.h"
#include "forwards.h"
#include "pros/imu.h"
#include "errno.h"

using namespace pros;
using namespace pros::c;

// Default gyro multiplier
// Calculation: 1.1 mV/dps = 1.365 quid/dps = 0.0007326007326007326 ... dpms/quid ~= 192.05>>18
#define GYRO_MULTIPLIER_DEFAULT 197

// Rate noise elimination threshold
// Default value (equivalence, in Pros implementaiton) is 188, which is 7 degrees / second
#define RATE_NOISE_LIMIT_ANALOG 128 //188


/*******************************************************************************
 * 
 * LegacyGyro class
 * 
 ******************************************************************************/
void LegacyGyro::Integrate()
{
    unsigned long time = millis();
    // Same as analogReadCalibratedHR(). // LSLed by 4 as offset
    int32_t reading = m_sensor.get_value() << 4;
    reading = int(reading) - int(m_calibValue);

    Assert(m_lastTime != 0);
    int timeDiff = time - m_lastTime;
    m_lastTime = time;

    Assert(timeDiff != 0);
    Assert(timeDiff <= 10); // that would be pretty catastrophic

    if (abs(reading) < m_limit)
        return;

    m_value += timeDiff * reading;
}

float LegacyGyro::GetAngle() const
{
    return m_value * m_multiplier;
}

LegacyGyro::LegacyGyro(unsigned char port, unsigned short multiplier)
    // Multiplier is (0.0007...<<18) dpms * DT ms * (reading<<4) quid = degrees<<22
    : m_multiplier(float(multiplier == 0 ? GYRO_MULTIPLIER_DEFAULT : multiplier) / (1 << 22)),
      m_sensor(port),
      m_calibValue(0),
      m_limit(RATE_NOISE_LIMIT_ANALOG)
{
    // Same as analogCalibrate()
    unsigned int Measurements = 0;
    unsigned int ActualMeasurement = 0;

    while (ActualMeasurement < 1024)
    {
        Measurements++;
        uint32_t value = m_sensor.get_value();
        if (value != 0 || Measurements >= 1024)
        {
            ActualMeasurement++;
            Assert(0 <= value && value <= 4096);
            m_calibValue += value;
        }
        task_delay(1);
    }

    m_calibValue /= (ActualMeasurement >> 4);
}

void LegacyGyro::ResetState()
{
    m_lastTime = millis() - 1;
}


/*******************************************************************************
 * 
 * GyroInertial class
 * 
 ******************************************************************************/
float GyroNothing::GetAngle() const{
    AssertSz(false, "GyroNothing is used");
}

/*******************************************************************************
 * 
 * GyroInertial class
 * 
 ******************************************************************************/
GyroInertial::GyroInertial(uint8_t port)
    : m_port(port)
{
    auto res = imu_reset(m_port);
    if (res != 1)
    {  
        ReportStatus(Log::Error, "Failed to init Inertial sensor: %d %d\n", res, errno);
        return;
    }
    // Right way to do it - test imu_get_status(m_port) for E_IMU_STATUS_CALIBRATING
    // But it always returns 18 right away
    delay(2200);
}

void GyroInertial::ResetState()
{
}

float GyroInertial::GetAngle() const
{
    return -imu_get_rotation(m_port);
}

/*******************************************************************************
 * 
 * GyroBoth class
 * 
 ******************************************************************************/
GyroBoth::GyroBoth()
{
}

void GyroBoth::Integrate()
{
    m_gyro.Integrate();
    m_gyro2.Integrate();
    m_gyroImu.Integrate();

    /*
    m_count++;
    if ((m_count % 10) == 0)
        ReportStatus(Log::Gyro, "%f\n", GetAngle());
    */
}

float GyroBoth::GetAngle() const
{
    float res = (1 * m_gyro.GetAngle() +  3 * m_gyro2.GetAngle()) / 4;
    return res;
}

void GyroBoth::ResetState()
{
    m_gyro.ResetState();
    m_gyro2.ResetState();
    m_gyroImu.ResetState();
}
