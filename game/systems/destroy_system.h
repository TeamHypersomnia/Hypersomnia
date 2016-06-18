#pragma once
#include "game/cosmos_reference.h"

class destroy_system : public cosmos_reference {
public:
	using cosmos_reference::cosmos_reference;

	void queue_children_of_queued_entities();
	void perform_deletions();
};