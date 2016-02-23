#pragma once
#include "entity_system/processing_system.h"
#include "../components/trigger_detector_component.h"

class trigger_detector_system : public augs::processing_system_templated<components::trigger_detector> {
public:
	using processing_system_templated::processing_system_templated;

	void consume_trigger_detector_presses();

	void post_trigger_requests_from_continuous_detectors();

	void find_trigger_collisions_and_send_confirmations();
};