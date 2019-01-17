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
#include <cmath>

// Default gyro multiplier
// Calculation: 1.1 mV/dps = 1.365 quid/dps = 0.0007326007326007326 ... dpms/quid ~= 192.05>>18
#define GYRO_MULTIPLIER_DEFAULT 193

// Rate noise elimination threshold
// Default value (equivalence, in Pros implementaiton) is 188, which is 7 degrees / second
#define RATE_NOISE_LIMIT_ANALOG 188 //138

void GyroWrapper::Integrate()
{
    unsigned long time = millis();
    // Same as analogReadCalibratedHR(). // LSLed by 4 as offset
    int32_t reading = analogRead(m_port) << 4;
    reading *= m_multiplier;
    reading = int(reading) - int(m_calibValue);

    if (m_lastTime == 0)
        m_lastTime = time-1;
    int timeDiff = time - m_lastTime;
    m_lastTime = time;

    // Assert(timeDiff != 0);
    // Assert(timeDiff <= 10); // that would be pretty catastrophic

    if (reading >= -m_limit && reading <= m_limit)
        return;

    // Multiplier is (0.0007...<<18) dpms * DT ms * (reading<<4) quid = degrees<<22
    // So we need to get from LSL22 to LSL10 = LSR12
    int32_t d = (timeDiff * reading + 0x800) >> 12;
    m_value += d;
}

GyroWrapper::GyroWrapper(unsigned char port, unsigned short multiplier)
    : m_multiplier(multiplier == 0 ? GYRO_MULTIPLIER_DEFAULT : multiplier),
        m_port(port)
{
    // Same as analogCalibrate() 
    uint32_t total = 0;
    const unsigned int Measurements = 1024;
    m_limit = RATE_NOISE_LIMIT_ANALOG * m_multiplier;

    for (unsigned int i = 0; i < Measurements; i++) {
        uint32_t value = analogRead(m_port);
        Assert(0 <= value && value <= 4096);
        total += value;
        taskDelay(1);
    }

    m_calibValue = ((uint64_t)total * m_multiplier) / (Measurements / 16);
}
