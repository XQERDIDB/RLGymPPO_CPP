#include "AdvancedObsPadder.h"
#include <cmath>
#include <vector>

namespace RLGSC {

AdvancedObsPadder::AdvancedObsPadder(int teamSize, bool expanding)
	: teamSize(teamSize), POS_STD(2300.0f), ANG_STD((float)M_PI), expanding(expanding) {}

void AdvancedObsPadder::Reset(const GameState& initialState) {
	// No-op for this OBS builder
}

FList AdvancedObsPadder::BuildOBS(const PlayerData& player, const GameState& state, const Action& prevAction) {
	FList obs;
	obs.reserve(237); // Pre-allocate memory for efficiency

	// Determine if we need inverted coordinates (Orange team)
	bool inverted = (player.team == Team::ORANGE);
	const PhysObj& ball = state.GetBallPhys(inverted);
	const auto& pads = state.GetBoostPads(inverted);

	// 1. Ball position / POS_STD (3 floats)
	obs += ball.pos / POS_STD;
	
	// 2. Ball linear velocity / POS_STD (3 floats)
	obs += ball.vel / POS_STD;
	
	// 3. Ball angular velocity / ANG_STD (3 floats)
	obs += ball.angVel / ANG_STD;

	// 4. Previous action (8 floats)
	for (int i = 0; i < 8; i++) {
		obs += prevAction[i];
	}

	// 5. Boost pad states (34 floats)
	for (int i = 0; i < CommonValues::BOOST_LOCATIONS_AMOUNT; i++) {
		obs += (float)pads[i];
	}

	// 6. Add player car data and get reference for relative calculations
	const PhysObj& playerCar = AddPlayerToOBS(obs, player, ball, inverted);

	// 7. Process allies and enemies
	std::vector<const PlayerData*> allies;
	std::vector<const PlayerData*> enemies;
	
	for (const auto& other : state.players) {
		if (other.carId == player.carId) continue;
		
		if (other.team == player.team) {
			allies.push_back(&other);
		} else {
			enemies.push_back(&other);
		}
	}

	// 8. Add allies data (up to teamSize-1)
	int allyCount = 0;
	for (const auto* ally : allies) {
		if (allyCount >= teamSize - 1) break;
		
		const PhysObj& otherCar = AddPlayerToOBS(obs, *ally, ball, inverted);
		
		// Extra info: relative position and velocity to player
		obs += (otherCar.pos - playerCar.pos) / POS_STD;
		obs += (otherCar.vel - playerCar.vel) / POS_STD;
		
		allyCount++;
	}
	
	// Pad remaining ally slots
	while (allyCount < teamSize - 1) {
		AddDummy(obs);
		allyCount++;
	}

	// 9. Add enemies data (up to teamSize)
	int enemyCount = 0;
	for (const auto* enemy : enemies) {
		if (enemyCount >= teamSize) break;
		
		const PhysObj& otherCar = AddPlayerToOBS(obs, *enemy, ball, inverted);
		
		// Extra info: relative position and velocity to player
		obs += (otherCar.pos - playerCar.pos) / POS_STD;
		obs += (otherCar.vel - playerCar.vel) / POS_STD;
		
		enemyCount++;
	}
	
	// Pad remaining enemy slots
	while (enemyCount < teamSize) {
		AddDummy(obs);
		enemyCount++;
	}

	// Handle expanding option
	if (expanding) {
		// This would typically expand dimensions for batch processing
		// For now, we'll just return the observation as-is since the Python
		// version uses np.expand_dims(np.concatenate(obs), 0) which adds a batch dimension
		// In C++, this is typically handled at a higher level
	}

	return obs;
}

void AdvancedObsPadder::AddDummy(FList& obs) {
	// Add dummy player data (26 floats for player info)
	obs += FList(3, 0.0f); // rel_pos
	obs += FList(3, 0.0f); // rel_vel
	obs += FList(3, 0.0f); // position
	obs += FList(3, 0.0f); // forward
	obs += FList(3, 0.0f); // up
	obs += FList(3, 0.0f); // linear_velocity
	obs += FList(3, 0.0f); // angular_velocity
	obs += FList{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // [boost, on_ground, has_flip, is_demoed, has_jump]
	
	// Add dummy relative position and velocity (6 floats)
	obs += FList(3, 0.0f); // relative position difference
	obs += FList(3, 0.0f); // relative velocity difference
}

const PhysObj& AdvancedObsPadder::AddPlayerToOBS(FList& obs, const PlayerData& player, const PhysObj& ball, bool inverted) {
	const PhysObj& playerCar = player.GetPhys(inverted);

	// Calculate relative position and velocity to ball
	Vec relPos = ball.pos - playerCar.pos;
	Vec relVel = ball.vel - playerCar.vel;

	// Add player data (26 floats total)
	obs += relPos / POS_STD;                    // 3 floats
	obs += relVel / POS_STD;                    // 3 floats
	obs += playerCar.pos / POS_STD;             // 3 floats
	obs += playerCar.rotMat.forward;            // 3 floats
	obs += playerCar.rotMat.up;                 // 3 floats
	obs += playerCar.vel / POS_STD;             // 3 floats
	obs += playerCar.angVel / ANG_STD;          // 3 floats
	
	// Player state (5 floats)
	obs += FList{
		player.boostFraction,
		(float)player.carState.isOnGround,
		(float)player.hasFlip,
		(float)player.carState.isDemoed,
		(float)player.hasJump
	};

	return playerCar;
}

} // namespace RLGSC
