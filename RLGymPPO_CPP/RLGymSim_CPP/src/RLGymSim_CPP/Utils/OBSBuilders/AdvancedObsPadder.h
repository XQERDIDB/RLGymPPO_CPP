#pragma once
#include "OBSBuilder.h"
#include <cmath>

namespace RLGSC {
	class AdvancedObsPadder : public OBSBuilder {
	public:
		int teamSize;
		float POS_STD, ANG_STD;
		bool expanding;

		// Constructor: Sets up the obs builder with a fixed team size for padding.
		// Use teamSize=3 for standard 1v1, 2v2, and 3v3 matches.
		AdvancedObsPadder(int teamSize = 3, bool expanding = false);

		virtual void Reset(const GameState& initialState) override;
		virtual FList BuildOBS(const PlayerData& player, const GameState& state, const Action& prevAction) override;

	private:
		// Adds a block of 32 zeros to pad a missing player.
		void AddDummy(FList& obs);

		// Adds a player's information to the observation list.
		const PhysObj& AddPlayerToOBS(FList& obs, const PlayerData& player, const PhysObj& ball, bool inv);
	};
}
