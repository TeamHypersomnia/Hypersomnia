#pragma once

class cosmos;
class step_state;

class destroy_system {
public:

	void queue_children_of_queued_entities(cosmos& cosmos, step_state& step);
	void perform_deletions(cosmos& cosmos, step_state& step);
};