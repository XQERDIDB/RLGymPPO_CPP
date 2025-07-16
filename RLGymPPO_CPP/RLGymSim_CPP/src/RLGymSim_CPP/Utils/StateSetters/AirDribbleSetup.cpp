// AirdribbleSetup.cpp
#include "AirdribbleSetup.h"
#include "../../Math.h"
#include "../CommonValues.h"
#include <cmath> // For std::atan2

RLGSC::GameState RLGSC::AirdribbleSetup::ResetState(Arena* arena) {
    // Standard setup: find one blue and one orange car
    Car* car_attack = nullptr;
    Car* car_defend = nullptr;
    for (Car* car : arena->_cars) {
        if (car->team == Team::BLUE) {
            car_attack = car;
        } else if (car->team == Team::ORANGE) {
            car_defend = car;
        }
    }

    // 1. Randomly decide which team will be on offense
    int team_fix = 1; // 1 for BLUE attacking, -1 for ORANGE attacking
    if (::Math::RandFloat() < 0.5f) {
        std::swap(car_attack, car_defend);
        team_fix = -1;
    }

    // 2. Randomly choose which wall to set up on (left or right)
    float wall_x;
    if (::Math::RandFloat() < 0.5f) {
        wall_x = CommonValues::SIDE_WALL_X - 700; // Right wall
    } else {
        wall_x = -CommonValues::SIDE_WALL_X + 700; // Left wall
    }

    // --- Car and Ball Velocity Setup ---
    // The car always drives away from the wall and towards the opponent's goal
    float car_vel_x = ::Math::RandFloat(1000, 1200) * ((wall_x < 0) ? -1 : 1);
    float car_vel_y = ::Math::RandFloat(50, 500) * team_fix;
    Vec car_velocity = Vec(car_vel_x, car_vel_y, 0);
    Vec ball_velocity = car_velocity * 1.2f;

    // --- Ball Position Setup ---
    // KEY CHANGE: The setup now always happens in the attacker's half of the field.
    // This makes the left/right wall choice truly independent.
    float ball_y_abs = ::Math::RandFloat(500, 3000); // Distance from the half-way line
    float ball_y = ball_y_abs * (team_fix * -1); // Place in attacker's half (Blue is -Y, Orange is +Y)
    
    float ball_x = (wall_x > 0) ? (wall_x - CommonValues::BALL_RADIUS - 10) : (wall_x + CommonValues::BALL_RADIUS + 10);
    float ball_z = 94;

    BallState bs = {};
    bs.pos = Vec(ball_x, ball_y, ball_z);
    bs.vel = ball_velocity * ballVelMult;
    if (ballZeroZ) {
        bs.vel.z = 0;
    }
    bs.angVel = Vec(0, 0, 0);
    arena->ball->SetState(bs);
    
    // --- Attacker Car Setup ---
    float car_x = ball_x - 500 * ((wall_x < 0) ? -1 : 1);
    float car_y = ball_y - car_vel_y * 0.5f;
    float car_z = 17;

    float car_rot_yaw = (car_vel_y == 0) ? 0 : -std::atan2(car_vel_y, car_vel_x);

    CarState cs_attack = {};
    cs_attack.pos = Vec(car_x, car_y, car_z);
    cs_attack.rotMat = Angle(0, car_rot_yaw, 0).ToRotMat();
    cs_attack.vel = car_velocity;
    cs_attack.angVel = Vec(0, 0.1f, 0);
    cs_attack.boost = ::Math::RandFloat(0.45f, 1.0f);
    car_attack->SetState(cs_attack);
    
    // --- Defender Setup ---
    for (Car* car : arena->_cars) {
        if (car == car_attack) continue;

        // Place defender in their own goal area
        CarState cs = {};
        cs.pos = Vec(::Math::RandFloat(-2000, 2000), team_fix * ::Math::RandFloat(4000, 5000), 17);
        cs.rotMat = Angle(0, -M_PI_2 * team_fix, 0).ToRotMat(); // Face towards the play
        cs.boost = ::Math::RandFloat(0.3f, 1.0f);
        car->SetState(cs);
    }

    return GameState(arena);
}