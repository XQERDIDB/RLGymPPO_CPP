// CeilingShotState.cpp
#include "CeilingShotState.h"
#include "../../Math.h"
#include "../CommonValues.h"
#include <cmath>

constexpr float DEG_TO_RAD = 3.14159265f / 180.0f;
constexpr float CEILING_HEIGHT = 2044.0f; // Rocket League ceiling height

RLGSC::GameState RLGSC::CeilingShotState::ResetState(Arena* arena) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Position ranges for ceiling shots - avoid corners and walls
    std::uniform_real_distribution<float> rand_x_dist(-2500.0f, 2500.0f);
    std::uniform_real_distribution<float> rand_y_dist(-3500.0f, 3500.0f);
    // Vary heights more - from lower to higher positions, max height 1850
    std::uniform_real_distribution<float> rand_z_dist(800.0f, 1850.0f);
    std::uniform_real_distribution<float> rand_speed_dist(m_speed_min, m_speed_max);
    
    Car* car_attack = nullptr;
    Car* car_defend = nullptr;
    for (Car* car : arena->_cars) {
        if (car->team == Team::BLUE) {
            car_attack = car;
        } else if (car->team == Team::ORANGE) {
            car_defend = car;
        }
    }

    int orange_fix = 1;
    if (::Math::RandFloat() < 0.5) {
        std::swap(car_attack, car_defend);
        orange_fix = -1;
    }

    float rand_x = rand_x_dist(gen);
    float rand_y = rand_y_dist(gen);
    float rand_z = rand_z_dist(gen);
    Vec desired_car_pos(rand_x, rand_y, rand_z);
    
    // Car flat with wheels aligned to ceiling
    float desired_yaw = (orange_fix * 90 + ::Math::RandFloat(-30, 30)) * DEG_TO_RAD;
    float desired_pitch = 0; // Completely flat
    float desired_roll = 180 * DEG_TO_RAD; // Upside down (wheels to ceiling)
    
    Angle desired_rotation(desired_yaw, desired_pitch, desired_roll);

    CarState cs_attack = {};
    cs_attack.pos = desired_car_pos;
    cs_attack.rotMat = desired_rotation.ToRotMat();
    cs_attack.boost = ::Math::RandFloat(50.0f, 100.0f);
    
    // Launch straight up towards ceiling at max speed
    cs_attack.vel = Vec(0, 0, 2299.99f);
    cs_attack.angVel = Vec(0, 0, 0);
    car_attack->SetState(cs_attack);

    // Position ball slightly in front of the car like in the almost version
    BallState bs = {};
    Vec ball_offset = Vec(
        ::Math::RandFloat(-30, 30), // Small lateral offset
        orange_fix * ::Math::RandFloat(250, 400), // In front of car
        ::Math::RandFloat(50, 120) // Slightly above the car
    );
    bs.pos = desired_car_pos + ball_offset;
    
    // Ensure ball spawn position is safe from ceiling
    if (bs.pos.z > 1600.0f) {
        bs.pos.z = ::Math::RandFloat(1400.0f, 1600.0f);
    }
    
    // Calculate safe upward velocity based on spawn height
    // Physics: max_height = spawn_z + (velocity_z^2) / (2 * gravity)
    // Rocket League gravity ≈ 650, ceiling = 2044
    float spawn_height = bs.pos.z;
    float max_safe_velocity_z = sqrt(2.0f * 650.0f * (2000.0f - spawn_height)); // 44uu safety margin
    float safe_velocity_z = ::Math::RandFloat(600.0f, std::min(max_safe_velocity_z, 900.0f));
    
    bs.vel = Vec(
        ::Math::RandFloat(-50, 50), // Small lateral velocity
        orange_fix * ::Math::RandFloat(400, 800), // Forward towards goal
        safe_velocity_z // Calculated safe upward velocity
    );
    bs.angVel = Vec(::Math::RandFloat(-3, 3), ::Math::RandFloat(-3, 3), ::Math::RandFloat(-3, 3));
    arena->ball->SetState(bs);

    // Set up defending and other cars
    for (Car* car : arena->_cars) {
        if (car == car_attack) {
            continue;
        }

        CarState cs = {};
        if (car == car_defend) {
            cs.pos = Vec(::Math::RandFloat(-800, 800), orange_fix * ::Math::RandFloat(4500, 5100), 17);
            cs.rotMat = Angle(0, ::Math::RandFloat(-M_PI/4, M_PI/4), 0).ToRotMat();
            cs.boost = 100.0f;
        } else {
            cs.pos = Vec(::Math::RandFloat(-1472, 1472), ::Math::RandFloat(-1984, 1984), 17);
            cs.rotMat = Angle(0, ::Math::RandFloat(-M_PI, M_PI), 0).ToRotMat();
            cs.boost = 100.0f;
        }
        car->SetState(cs);
    }

    return GameState(arena);
}