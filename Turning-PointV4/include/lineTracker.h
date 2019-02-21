#pragma once


class LineTracker
{
public:
    LineTracker(unsigned int port);
    void Reset();
    void Update();
    unsigned int GetWhiteLineDistance(bool pop);
    bool HasWhiteLine();

protected:
    enum class Status
    {
        None,
        HitWhite,
        HitBlack,
    };
    void Push(bool white);
    void Shift(unsigned int elements);
    int IndexForWhiteLine();

protected:
    constexpr static unsigned int WhiteLevel = 2500;
    constexpr static unsigned int BlackLevel = 3000;
    constexpr static unsigned int TimeNone = 0;

    unsigned int m_times[8];
    unsigned int m_timesIndex = 0;

    unsigned int m_port = 0;

    Status m_status = Status::None;
};
