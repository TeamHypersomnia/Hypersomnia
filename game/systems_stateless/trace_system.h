#pragma once

class logic_step;

class trace_system {
public:
	void lengthen_sprites_of_traces(logic_step&) const;
	void destroy_outdated_traces(logic_step&) const;
	void spawn_finishing_traces_for_destroyed_objects(logic_step&) const;
};