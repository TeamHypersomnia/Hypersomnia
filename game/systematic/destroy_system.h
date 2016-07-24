#pragma once

class fixed_step;

class destroy_system {
public:
	void queue_children_of_queued_entities(fixed_step& step);
	void perform_deletions(fixed_step&);
};