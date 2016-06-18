#pragma once
#include "scene_builder.h"
#include "game/entity_id.h"

namespace scene_builders {
	struct one_entity : public scene_builder {
		void load_resources() override;
		void populate_world_with_entities(cosmos world) override;
		void perform_logic_step(cosmos world) override;
		void execute_drawcalls_for_camera(messages::camera_render_request_message) override;
	};
}