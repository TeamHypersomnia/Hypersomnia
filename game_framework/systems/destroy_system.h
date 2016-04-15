#pragma once
#include "entity_system/processing_system.h"

using namespace augs;

class destroy_system : public event_only_system {
public:
	using event_only_system::event_only_system;

	void purge_queue_of_duplicates();
	void delete_queued_entities();
	void purge_message_queues_of_dead_entities();
};