#pragma once

class cosmos;
class fixed_step;

class trigger_detector_system {
public:
	void consume_trigger_detector_presses(fixed_step&) const;
	void post_trigger_requests_from_continuous_detectors(fixed_step&) const;
	void send_trigger_confirmations(fixed_step&) const;
};