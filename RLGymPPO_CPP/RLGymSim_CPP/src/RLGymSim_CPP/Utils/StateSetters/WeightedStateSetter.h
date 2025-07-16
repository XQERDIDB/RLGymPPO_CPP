// WeightedStateSetter.h
#pragma once

#include <vector>
#include <random>
#include <memory>
#include <string>

#include "StateSetter.h"
#include "KickoffState.h"
#include "FlickState.h"
#include "AerialState.h"
#include "AirdribbleSetup.h"
#include "RandomState.h"

namespace RLGSC {

class WeightedStateSetter : public StateSetter {
public:
    WeightedStateSetter(int gameMode) {
        // Here we define the probability for each scenario.
        // They should all add up to 1.0 (or 100%).
        // You can tune these values to focus training on specific skills.
        double kickoffWeight;
        double flickWeight;
        double aerialWeight;
        double airdribbleSetupWeight;
        double randomWeight;

        // For now, we will use the same balanced weights for all game modes,
        // but you can customize them inside the switch statement later.
        switch (gameMode) {
            case 1: // 1v1
            case 2: // 2v2
            case 3: // 3v3
            default:
                kickoffWeight         = 0.20; // 10% chance for a kickoff
                flickWeight           = 0.20; // 25% chance for a dribble/flick setup
                aerialWeight          = 0.30; // 25% chance for an aerial setup
                airdribbleSetupWeight = 0.30; // 20% chance for a wall air dribble setup
                randomWeight          = 0.00; // 20% chance for a chaotic random state
                break;
        }

        // Add all the state setters and their corresponding weights to the list
        stateSetters.emplace_back(std::make_pair(std::make_unique<KickoffState>(), kickoffWeight));
        stateSetters.emplace_back(std::make_pair(std::make_unique<FlickState>(), flickWeight));
        stateSetters.emplace_back(std::make_pair(std::make_unique<AerialState>(), aerialWeight));
        stateSetters.emplace_back(std::make_pair(std::make_unique<AirdribbleSetup>(), airdribbleSetupWeight));
        stateSetters.emplace_back(std::make_pair(std::make_unique<RandomState>(true, true, false), randomWeight));
        
        initializeWeights();
    }

    virtual GameState ResetState(Arena* arena) override {
        // This logic is now restored: it randomly selects a state setter
        // from the list based on the probabilities (weights) provided.
        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<int> distribution(weights.begin(), weights.end());
        int selectedIndex = distribution(gen);

        // Call the ResetState function of the randomly selected state setter
        return stateSetters[selectedIndex].first->ResetState(arena);
    }

private:
    std::vector<std::pair<std::unique_ptr<StateSetter>, double>> stateSetters;
    std::vector<double> weights;

    void initializeWeights() {
        weights.clear();
        for (const auto& pair : stateSetters) {
            weights.push_back(pair.second);
        }
    }
};

}