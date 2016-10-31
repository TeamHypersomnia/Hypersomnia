#pragma once

namespace messages {
	struct health_event;
}

class logic_step;

class sentience_system {
	void consume_health_event(messages::health_event, logic_step&) const;

public:
	void apply_damage_and_generate_health_events(logic_step&) const;
	void cooldown_aimpunches(logic_step&) const;
	void set_borders(logic_step&) const;
	void regenerate_values(logic_step&) const;
};