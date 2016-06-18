#pragma once

class trace_system {
public:
	void lengthen_sprites_of_traces();
	void destroy_outdated_traces();
	void spawn_finishing_traces_for_destroyed_objects();
};