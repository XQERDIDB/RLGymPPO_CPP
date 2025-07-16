// AirdribbleSetup.h
#pragma once
#include "StateSetter.h"

namespace RLGSC {
    class AirdribbleSetup : public StateSetter {
    public:
        // Constructor to match the Python version's parameters
        AirdribbleSetup(float ballVelMult = 1.0f, bool ballZeroZ = false)
            : ballVelMult(ballVelMult), ballZeroZ(ballZeroZ) {}

        virtual GameState ResetState(Arena* arena) override;

    private:
        float ballVelMult;
        bool ballZeroZ;
    };
}