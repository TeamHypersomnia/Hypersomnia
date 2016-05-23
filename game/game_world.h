#pragma once
#include "entity_system/world.h"
#include "window_framework/window.h"

namespace augs {
	struct lua_state_wrapper;
}

class game_world : public augs::world {
public:
	game_world(augs::overworld& parent_overworld);

	void register_types_of_messages_components_systems();
	
	void call_drawing_time_systems();
	void perform_logic_step();
	void rendering_time_creation_callbacks();
	void creation_callbacks();
	void destruction_callbacks();
	void restore_transforms_after_drawing();

	std::wstring world_summary(bool) const final;

	void bind_this_to_lua_global(augs::lua_state_wrapper&, std::string global);
};