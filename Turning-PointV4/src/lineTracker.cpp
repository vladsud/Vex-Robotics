#include "lineTracker.h"
#include "main.h"
#include "cycle.h"

LineTracker::LineTracker(unsigned int port)
    : m_port(port)
{
    Reset();
}

void LineTracker::Reset()
{
    m_status = Status::None;
    for (unsigned int i = 0; i < CountOf(m_times); i++)
        m_times[i] = TimeNone;
    m_timesIndex = 0;
}

void LineTracker::Shift(unsigned int elements)
{
    for (unsigned int i = elements; i < CountOf(m_times); i++)
        m_times[i-elements] = m_times[i];

    Assert(m_timesIndex >= elements);
    m_timesIndex -= elements;
}

void LineTracker::Push(bool white)
{
    if (m_timesIndex == CountOf(m_times))
        Shift(1);
    m_times[m_timesIndex] = GetMain().drive.m_distance;
    ReportStatus("Line hit: port=%d, dist=%d\n", m_port, m_times[m_timesIndex]);
    m_timesIndex++;
}

void LineTracker::Update()
{
    unsigned int value = analogRead(m_port);

    StaticAssert(WhiteLevel < BlackLevel);

    if (m_status != Status::HitBlack && value > BlackLevel)
    {
        m_status = Status::HitBlack;
        Push(false/*white*/);
    }

    if (m_status != Status::HitWhite && value < WhiteLevel)
    {
        m_status = Status::HitWhite;
        Push(true/*white*/);
    }
}

int LineTracker::IndexForWhiteLine()
{
   // if we crossed one white line, we should have this state:
    // m_times: black-white-black
    // m_status == HitBlack
    unsigned int index = 0;
    if (((m_timesIndex % 2) == 1) == (m_status == Status::HitBlack))
        index++;

    if (index + 2 > m_timesIndex)
        return -1;

    return index;
 }

unsigned int LineTracker::GetWhiteLineDistance(bool pop)
{
    int index = IndexForWhiteLine();
    Assert(index >= 0);
    if (index == -1)
        return 0;

    unsigned int result = (m_times[index] + m_times[index+1]) / 2;
    if (pop)
        Shift(index+2);
    return result;
}

bool LineTracker::HasWhiteLine()
{
    Update();
    return IndexForWhiteLine() >= 0;
}
