#pragma once
#include "main.h"

class Cubetray
{
    Direction m_direction = Direction::None;
    bool m_initialize = false;
    bool m_unload = false;

    unsigned int m_count = 0;
    int m_lastPos;
    

    void Initialize();
    void Unload();

  public:
    void Update();
};