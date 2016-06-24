#pragma once

class cosmos;
class step_state;

class destroy_system {
public:

	void queue_children_of_queued_entities(fixed_step& step);
	void perform_deletions();
};