#pragma once

#include "scene_builder.h"
#include "utilities/entity_system/entity_id.h"

namespace scene_builders {
	struct testbed : public scene_builder {
		bool keep_drawing = false;

		void initialize(augs::world& world) override;
		void perform_logic_step(augs::world& world) override;
		void drawcalls_after_all_cameras(augs::world& world) override;

		void execute_drawcalls(messages::camera_render_request_message) override;
	};
}