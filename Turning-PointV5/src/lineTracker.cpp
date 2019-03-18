#include "lineTracker.h"
#include "main.h"
#include "cycle.h"
#include <cmath>
#include <cstdio>

#include "pros/adi.h"

using namespace pros::c;

LineTracker::LineTracker(unsigned int port)
    : m_port(port)
{
    ReportStatus("Line %d init: %d\n", m_port, adi_analog_read(m_port));
    ResetCore();
}

void LineTracker::ResetCore()
{
    m_status = Status::None;
    for (unsigned int i = 0; i < CountOf(m_times); i++)
        m_times[i] = 0;
    m_timesIndex = 0;
    m_minvalue = 4000;
}

void LineTracker::Reset()
{
    ResetCore();
    // It's important to do update right away, to detect if we are starting on white/red.
    Assert(GetMain().drive.m_distance == 0);
    Update();
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
    m_timesIndex++;
}

void LineTracker::Update()
{
    int value = adi_analog_read(m_port);
    if (m_minvalue > value)
        m_minvalue = value;

    // ReportStatus("L%d: %d\n", m_port, value);
    StaticAssert(WhiteLevel < BlackLevel);

    if (m_status != Status::HitBlack && value > BlackLevel)
    {
        Push(false/*white*/);
        m_status = Status::HitBlack;
    }

    if (m_status != Status::HitWhite && value < WhiteLevel)
    {
        Push(true/*white*/);
        if (m_times[m_timesIndex-1] == 0)
        {
            m_timesIndex--;
            // ReportStatus("   Starting on white: port=%d, brightness=%d\n", m_port, value);
        }
        else
        {
            // ReportStatus("   White line hit: port=%d, dist=%d, brightness=%d\n", m_port, m_times[m_timesIndex-1], value);
        }
        m_status = Status::HitWhite;
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

    if (index + 1 > m_timesIndex)
        return -1;

    return index;
 }

unsigned int LineTracker::GetWhiteLineDistance(bool pop)
{
    int index = IndexForWhiteLine();
    Assert(index >= 0);
    if (index == -1)
        return 0;

    // unsigned int result = (m_times[index] + m_times[index+1]) / 2;
    unsigned int result = m_times[index];
    if (pop)
        Shift(index+1);
    return result;
}

bool LineTracker::HasWhiteLine(int shouldHaveTravelled)
{
    int index = IndexForWhiteLine();
    if (index == -1)
        return false;

    if (abs(shouldHaveTravelled - (int)m_times[index]) > 400)
    {
        ReportStatus("Single line correction IGNORED: Travelled: %d, should: %d\n",
            m_times[index], shouldHaveTravelled);
        Shift(index+1);
        return false;
    }

    return true;
}
