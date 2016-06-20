#pragma once

class cosmos;
class step_state;

class trigger_detector_system {
public:
	void consume_trigger_detector_presses();

	void post_trigger_requests_from_continuous_detectors(cosmos&, step_state&);

	void send_trigger_confirmations();
};