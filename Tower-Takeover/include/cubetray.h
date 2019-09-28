#pragma once
#include "main.h"

class Cubetray
{
    Direction m_direction = Direction::None;
    bool m_initialize = true;
    bool m_unload = false;

    unsigned int m_count = 0;
    int m_lastPos;
    
    int m_initializationDistance = 100;

    void Initialize();
    void Unload();

  public:
    void Update();
};