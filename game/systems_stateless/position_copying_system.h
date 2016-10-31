#pragma once

class cosmos;
class logic_step;

class position_copying_system {
public:
	void update_transforms(logic_step& step);
};