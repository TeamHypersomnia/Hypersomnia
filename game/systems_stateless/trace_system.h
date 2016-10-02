#pragma once

class fixed_step;

class trace_system {
public:
	void lengthen_sprites_of_traces(fixed_step&) const;
	void destroy_outdated_traces(fixed_step&) const;
	void spawn_finishing_traces_for_destroyed_objects(fixed_step&) const;
};