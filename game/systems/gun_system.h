#pragma once

class physics_system;
class cosmos;
class step_state;

class gun_system {
public:
	
	void consume_gun_intents(cosmos& cosmos, step_state& step);
	void launch_shots_due_to_pressed_triggers(cosmos& cosmos, step_state& step);
};