#pragma once
#include "entity_system/processing_system.h"
#include "../components/children_component.h"

using namespace augs;


class destroy_system : public processing_system_templated<components::children> {
public:
	using processing_system_templated::processing_system_templated;

	void add(entity_id) override {}
	void remove(entity_id) override {}

	void delete_queued_entities();
};