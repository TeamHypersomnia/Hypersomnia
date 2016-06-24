#pragma once

class cosmos;
class fixed_step;

class damage_system {
public:

	void destroy_colliding_bullets_and_send_damage(fixed_step& step);
	void destroy_outdated_bullets(fixed_step& step);
};