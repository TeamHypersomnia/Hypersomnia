#pragma once
#include "entity_system/world.h"
#include "window_framework/window.h"

namespace augs {
	struct lua_state_wrapper;
}

class game_world : public augs::world {
public:
	game_world(augs::overworld&);
	
	void bind_this_to_lua_global(augs::lua_state_wrapper&, std::string global);

	void register_messages_components_systems();
	
	void perform_logic_step();
	
	void draw();
	void restore_transforms_after_rendering();
};