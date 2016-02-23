#pragma once

#include "scene_builder.h"
#include "augs/entity_system/entity_id.h"

namespace scene_builders {
	struct testbed : public scene_builder {
		std::vector<augs::entity_id> characters;
		unsigned current_character = 0;
		augs::entity_id world_camera;

		bool keep_drawing = false;

		void initialize(augs::world& world) override;
		void perform_logic_step(augs::world& world) override;
		void drawcalls_after_all_cameras(augs::world& world) override;

		void execute_drawcalls_for_camera(messages::camera_render_request_message) override;
	};
}