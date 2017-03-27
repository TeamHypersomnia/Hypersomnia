#pragma once
#include "setup_base.h"

typedef int resource_id_type;

struct play_scene {
	// GEN INTROSPECTOR struct play_scene
	resource_id_type id = 0;
	double at_time = 0.0;
	// END GEN INTROSPECTOR
};

struct play_sound {
	// GEN INTROSPECTOR struct play_sound
	resource_id_type id = 0;
	double at_time = 0.0;
	// END GEN INTROSPECTOR
};

struct focus_entity {
	// GEN INTROSPECTOR struct focus_entity
	entity_guid guid;
	double at_time = 0.0;
	// END GEN INTROSPECTOR
};

class game_window;

class choreographic_setup : public setup_base {
public:
	void process(
		const config_lua_table& cfg, 
		game_window&
	);
};