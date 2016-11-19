#pragma once

class cosmos;
#include "game/transcendental/step_declaration.h"

class trigger_detector_system {
public:
	void consume_trigger_detector_presses(logic_step&) const;
	void post_trigger_requests_from_continuous_detectors(logic_step&) const;
	void send_trigger_confirmations(logic_step&) const;
};