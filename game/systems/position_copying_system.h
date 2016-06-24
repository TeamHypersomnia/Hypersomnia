#pragma once

class cosmos;
class fixed_step;

class position_copying_system {
public:
	void update_transforms(fixed_step& step);
};