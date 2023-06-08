#pragma once

class cosmos;
#include "game/cosmos/step_declaration.h"

class missile_system {
public:

	void advance_penetrations(const logic_step step);

	void ricochet_missiles(const logic_step step);
	void detonate_colliding_missiles(const logic_step step);
	void detonate_expired_missiles(const logic_step step);
};