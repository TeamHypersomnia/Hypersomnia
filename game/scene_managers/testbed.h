#pragma once
#include "game/entity_id.h"

class fixed_step;
class cosmos;

namespace scene_managers {
	struct testbed {
		std::vector<entity_id> characters;
		
		unsigned current_character = 0;
		entity_id world_camera;

		std::vector<entity_id> draw_bodies;

		bool show_profile_details = false;
		bool keep_drawing = false;

		void populate_world_with_entities(fixed_step&);
		
		void pre_solve(fixed_step&);
		void post_solve(cosmos&, const step_state&);

		void drawcalls_after_all_cameras(fixed_step&);

		void execute_drawcalls_for_camera(messages::camera_render_request_message);
	};
}