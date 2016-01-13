#pragma once
#include "entity_system/processing_system.h"

class trigger_detector_system : public augs::event_only_system {
public:
	using event_only_system::event_only_system;

	void find_trigger_collisions_and_send_confirmations();
};