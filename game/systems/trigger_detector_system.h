#pragma once
#include "game/processing_system_with_cosmos_reference.h"
#include "game/components/trigger_query_detector_component.h"

class trigger_detector_system : public augs::processing_system_templated<components::trigger_query_detector> {
public:
	using processing_system_templated::processing_system_templated;

	void consume_trigger_detector_presses();

	void post_trigger_requests_from_continuous_detectors();

	void send_trigger_confirmations();
};