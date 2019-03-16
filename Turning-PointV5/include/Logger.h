#pragma once
#include "main.h"

enum class Diagnostics
{
    General,
    Drive,
    Intake,
    Angle,
    Autonomous,
};

bool PrintDiagnostics(Diagnostics diag);

enum class LogEntry : char
{
    Position = 0,
    Driving,
    Max,
};

#ifdef OFFICIAL_RUN

class Logger
{
  public:
    void Dump() {}
    void Log(LogEntry, int, int, int) {}
};

#else // OFFICIAL_RUN

enum class LogArgTypes
{
    End,
    SignedByte,
    SignedShort,
};

const int LogArgsMax = 6;
using Byte = unsigned short;

class Logger
{
    const unsigned int TimeMultimplier = 25;
    const unsigned int TotalBytes = 16 * 1024;

    Byte *m_data = nullptr;
    Byte *m_dataCurr = nullptr;
    unsigned long m_lastTime;
    bool m_recording = true;

  private:
    static signed char ToLogByte(int);
    static signed short int ToLogWord(int);
    Byte GetTime();
    bool AllocateBuffer(const LogArgTypes *types);
    void Pack(LogArgTypes type, int arg);
    static int UnPack(Byte *&buffer, LogArgTypes type);

  public:
    void Dump();
    void Log(LogEntry log, int a, int b, int c);
};

#endif // OFFICIAL_RUN

Logger &GetLogger();
