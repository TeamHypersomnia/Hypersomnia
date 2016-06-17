#pragma once
#include "game/processing_system_with_cosmos_reference.h"

using namespace augs;

class destroy_system : public processing_system_with_cosmos_reference {
public:
	using processing_system_with_cosmos_reference::processing_system_with_cosmos_reference;

	void queue_children_of_queued_entities();
	void perform_deletions();
};