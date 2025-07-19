// CeilingShotState.h
#pragma once
#include "StateSetter.h"

namespace RLGSC {
    class CeilingShotState : public StateSetter {
    public:
        CeilingShotState(float min_height = 1500.0f, float max_height = 1900.0f, 
                        float speed_min = 1200.0f, float speed_max = 1600.0f)
            : m_min_height(min_height), m_max_height(max_height), 
              m_speed_min(speed_min), m_speed_max(speed_max) {}

        virtual GameState ResetState(Arena* arena);

    private:
        float m_min_height;
        float m_max_height;
        float m_speed_min;
        float m_speed_max;
    };
}