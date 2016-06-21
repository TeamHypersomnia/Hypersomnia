#pragma once

class cosmos;
class step_state;

class position_copying_system {
public:
	void update_transforms(cosmos& cosmos, step_state& step);
};