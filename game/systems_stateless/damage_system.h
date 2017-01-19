#pragma once

class cosmos;
#include "game/transcendental/step_declaration.h"

class damage_system {
public:

	void destroy_colliding_bullets_and_send_damage(const logic_step step);
	void destroy_outdated_bullets(const logic_step step);
};