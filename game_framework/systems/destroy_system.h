#pragma once
#include "entity_system/processing_system.h"
#include "../components/children_component.h"

using namespace augs;

class destroy_system : public event_only_system {
public:
	using event_only_system::event_only_system;

	void delete_queued_entities();
};