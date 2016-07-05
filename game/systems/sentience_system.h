#pragma once

namespace messages {
	struct health_event;
}

class fixed_step;

class sentience_system {
	void consume_health_event(messages::health_event, fixed_step&) const;

public:
	void apply_damage_and_generate_health_events(fixed_step&) const;
	void cooldown_aimpunches(fixed_step&) const;
	void set_borders(fixed_step&) const;
	void regenerate_values(fixed_step&) const;
};