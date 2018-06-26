#pragma once

class cosmos;
#include "game/transcendental/step_declaration.h"

class missile_system {
public:

	void ricochet_missiles(const logic_step step);
	void detonate_colliding_missiles(const logic_step step);
	void detonate_expired_missiles(const logic_step step);
};