#pragma once
#include "scene_builder.h"
#include "game/entity_id.h"

namespace scene_builders {
	struct testbed : public scene_builder {
		std::vector<entity_id> characters;
		unsigned current_character = 0;
		entity_id world_camera;

		std::vector<entity_id> draw_bodies;

		bool show_profile_details = false;
		bool keep_drawing = false;

		void load_resources() override;
		void populate_world_with_entities(cosmos&) override;
		void perform_logic_step(cosmos&) override;
		void drawcalls_after_all_cameras(cosmos&) override;

		void execute_drawcalls_for_camera(messages::camera_render_request_message) override;
	};
}