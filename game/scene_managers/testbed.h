#pragma once
#include "game/entity_id.h"

class step_state;
class cosmos;

namespace scene_managers {
	struct testbed {
		std::vector<entity_id> characters;
		
		unsigned current_character = 0;
		entity_id world_camera;

		std::vector<entity_id> draw_bodies;

		bool show_profile_details = false;
		bool keep_drawing = false;

		void populate_world_with_entities(cosmos&, step_state&);
		void perform_logic_step(cosmos&, step_state&);
		void drawcalls_after_all_cameras(cosmos&, step_state&);

		void execute_drawcalls_for_camera(messages::camera_render_request_message);
	};
}