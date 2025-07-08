#include "AdvancedObsPadder.h"
#include <cmath>
#include <vector>

namespace RLGSC {

AdvancedObsPadder::AdvancedObsPadder(int teamSize, bool expanding)
	: teamSize(teamSize), POS_STD(2300.0f), ANG_STD(3.1415926f), expanding(expanding) {}

void AdvancedObsPadder::Reset(const GameState& initialState) {
	// No-op for this OBS builder
}

FList AdvancedObsPadder::BuildOBS(const PlayerData& player, const GameState& state, const Action& prevAction) {
	FList obs;
	obs.reserve(237); // Pre-allocate memory for efficiency

	bool inv = player.team == Team::ORANGE;
	const auto& ball = state.GetBallPhys(inv);
	const auto& pads = state.GetBoostPads(inv);

	// 1. Ball Information (9 floats)
	obs += ball.pos / POS_STD;
	obs += ball.vel / POS_STD;
	obs += ball.angVel / ANG_STD;

	// 2. Previous Action (8 floats) - Hardcoded to 8 to match DiscreteAction
	for (int i = 0; i < 8; i++)
		obs += prevAction[i];

	// 3. Boost Pad States (34 floats)
	for (int i = 0; i < CommonValues::BOOST_LOCATIONS_AMOUNT; i++)
		obs += (float)pads[i];

	// 4. Self Player Data (26 floats)
	const PhysObj& playerCar = AddPlayerToOBS(obs, player, ball, inv);

	// 5. Allies and Enemies Data (160 floats total)
	std::vector<PlayerData> allies_real, enemies_real;
	for (const auto& other : state.players) {
		if (other.carId == player.carId) continue;
		(other.team == player.team ? allies_real : enemies_real).push_back(other);
	}

	FList allies_obs, enemies_obs;

	// Add real allies and pad the rest
	int allyCount = 0;
	for (const auto& ally : allies_real) {
		if (allyCount >= teamSize - 1) break;
		const PhysObj& otherCar = AddPlayerToOBS(allies_obs, ally, ball, inv);
		allies_obs += (otherCar.pos - playerCar.pos) / POS_STD;
		allies_obs += (otherCar.vel - playerCar.vel) / POS_STD;
		allyCount++;
	}
	while (allyCount < teamSize - 1) {
		AddDummy(allies_obs);
		allyCount++;
	}

	// Add real enemies and pad the rest
	int enemyCount = 0;
	for (const auto& enemy : enemies_real) {
		if (enemyCount >= teamSize) break;
		const PhysObj& otherCar = AddPlayerToOBS(enemies_obs, enemy, ball, inv);
		enemies_obs += (otherCar.pos - playerCar.pos) / POS_STD;
		enemies_obs += (otherCar.vel - playerCar.vel) / POS_STD;
		enemyCount++;
	}
	while (enemyCount < teamSize) {
		AddDummy(enemies_obs);
		enemyCount++;
	}

	obs += allies_obs;
	obs += enemies_obs;

	// Final check: obs.size() should be 237
	return obs;
}

void AdvancedObsPadder::AddDummy(FList& obs) {
	// A full "other player" block consists of their own data (26 floats)
	// plus their relative position and velocity to the agent (6 floats).
	// Total size is 32. We add 32 zeros.
	obs += FList(32);
}

const PhysObj& AdvancedObsPadder::AddPlayerToOBS(FList& obs, const PlayerData& player, const PhysObj& ball, bool inv) {
	const PhysObj& car = player.GetPhys(inv);

	Vec relPosToBall = ball.pos - car.pos;
	Vec relVelToBall = ball.vel - car.vel;

	// This is the player's own data block (26 floats)
	obs += relPosToBall / POS_STD;
	obs += relVelToBall / POS_STD;
	obs += car.pos / POS_STD;
	obs += car.rotMat.forward;
	obs += car.rotMat.up;
	obs += car.vel / POS_STD;
	obs += car.angVel / ANG_STD;
	obs += FList{
		player.boostFraction,
		(float)player.carState.isOnGround,
		(float)player.hasFlip,
		(float)player.carState.isDemoed,
		(float)player.hasJump
	};

	return car;
}

} // namespace RLGSC