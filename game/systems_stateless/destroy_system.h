#pragma once

class logic_step;

class destroy_system {
public:
	void queue_children_of_queued_entities(logic_step& step);
	void perform_deletions(logic_step&);
};