#include "logger.h"
#include <bits/move.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pros/rtos.h"

using namespace pros;

// #define LOGGING


bool PrintDiagnostics(Diagnostics diag)
{
    switch (diag)
    {
    // case Diagnostics::Autonomous:
    // case Diagnostics::General:
    // case Diagnostics::Drive:
    // case Diagnostics::Angle:
    //  return true;
    default:
        return false;
    }
}

static const char *s_LogEntryNames[(int)LogEntry::Max] = {
    "\nPosition: ",
    "\nDriving: "};

LogArgTypes PositionArgs[] = {LogArgTypes::SignedByte, LogArgTypes::SignedByte, LogArgTypes::SignedShort, LogArgTypes::End};
LogArgTypes DrivingArgs[] = {LogArgTypes::End};

static const LogArgTypes *s_Types[(int)LogEntry::Max] = {
    PositionArgs,
    DrivingArgs,
};

void Logger::Dump()
{
    char buffer[sizeof(int) * 3 + 1];
    Byte *data = m_data;

    int count = 0;
    while (data < m_dataCurr)
    {
        count++;
        int entry = *data++;
        printf(s_LogEntryNames[entry]);

        Byte time = (Byte)*data++;
        printf(itoa(time, buffer, 10));

        const LogArgTypes *types = s_Types[entry];
        while (*types != LogArgTypes::End)
        {
            printf(", ");
            printf(itoa(UnPack(data, *types), buffer, 10));
            types++;
        }
    }

    Assert(data == m_dataCurr);

    puts("");

    printf("%d records, %d bytes\n", count, int(m_dataCurr - m_data));

    free(m_data);
    m_data = nullptr;
    m_dataCurr = nullptr;

    m_recording = false;
}

bool Logger::AllocateBuffer(const LogArgTypes *types)
{
    if (!m_recording)
        return false;

    if (m_data == nullptr)
    {
        m_data = new Byte[TotalBytes];
        if (m_data == nullptr)
        {
            AssertSz(false, "Can't allocate");
            m_recording = false;
            return false;
        }
        m_dataCurr = m_data;
    }

    if (m_dataCurr + (LogArgsMax + 2) * sizeof(int) >= m_data + TotalBytes)
    {
        AssertSz(false, "Log is full!");
        m_recording = false;
        return false;
    }

    return true;
}

void Logger::Pack(LogArgTypes type, int arg)
{
    switch (type)
    {
    case LogArgTypes::SignedByte:
        *m_dataCurr++ = ToLogByte(arg);
        break;
    case LogArgTypes::SignedShort:
        *(signed short *)m_dataCurr = ToLogWord(arg);
        m_dataCurr += 2;
        break;
    case LogArgTypes::End:
    default:
        AssertSz(false, "Logger::Pack(): incorrect argument type");
    }
}

int Logger::UnPack(Byte *&buffer, LogArgTypes type)
{
    switch (type)
    {
    case LogArgTypes::SignedByte:
        return (signed char)*buffer++;
    case LogArgTypes::SignedShort:
    {
        int res = *(signed short *)buffer;
        buffer += 2;
        return res;
    }
    case LogArgTypes::End:
    default:
        AssertSz(false, "Logger::Pack(): incorrect argument type");
        return -1;
    }
}

Byte Logger::GetTime()
{
    unsigned long time = pros::c::millis();
    if (m_data == m_dataCurr)
        m_lastTime = time;
    time -= m_lastTime;
    m_lastTime += time;
    if (time > 255)
        return 255;
    return time;
}

signed char Logger::ToLogByte(int value)
{
    if (value > 127)
        return 127;
    if (value < -128)
        return -128;
    return (signed char)value;
}

signed short int Logger::ToLogWord(int value)
{
    if (value > 0x7fff)
        return 0x7fff;
    if (value < -0x8000)
        return -0x8000;
    return (signed short int)value;
}

void Logger::Log(LogEntry log, int arg1, int arg2, int arg3)
{
#ifdef LOGGING
    const LogArgTypes *types = s_Types[(int)log];
    Assert(types[3] == LogArgTypes::End);

    if (!AllocateBuffer(types))
        return;

    *m_dataCurr++ = (Byte)log;
    *m_dataCurr++ = GetTime();

    Pack(*types++, arg1);
    Pack(*types++, arg2);
    Pack(*types++, arg3);
#endif
}
