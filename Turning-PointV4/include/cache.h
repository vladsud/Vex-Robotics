#pragma once
#include <bits/move.h>

template <typename Input, typename Output, typename Funct, int TSize> 
class Cache
{
    struct Record
    {
        Input in;
        Output out;
        unsigned int count;
    };
    Record m_data[TSize];
    int m_size = 0;
    Funct m_funct;

public:
    Cache(Funct&& funct)
        : m_funct(std::forward(funct))
    {
    }

    Output Calc(Input in)
    {
        Record* rec = m_data;
        unsigned int countMin = 0xffffffff;
        int indexMin = 0;
        for (int i = 0; i< m_size; i++)
        {
            if (rec->in == in)
            {
                rec->count++;
                return rec->out;
            }
            if (countMin > rec->count)
            {
                countMin = rec->count;
                indexMin = i;
            }
            rec++;
        }

        if (m_size != TSize)
        {
            countMin = m_size;
            m_size++;
        }
        rec = m_data + countMin;
        rec->in = in;
        rec->out = Funct(in);
        rec->count = 1;
        return rec->out;
    }
};
