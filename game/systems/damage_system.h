#pragma once

class cosmos;
class step_state;

class damage_system {
public:

	void destroy_colliding_bullets_and_send_damage(cosmos& cosmos, step_state& step);
	void destroy_outdated_bullets(cosmos& cosmos, step_state& step);
};