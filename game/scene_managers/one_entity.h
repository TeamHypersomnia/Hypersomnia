#pragma once
#include "scene_manager.h"
#include "game/entity_id.h"

namespace scene_managers {
	struct one_entity : public scene_manager {
		void load_resources() override;
		void populate_world_with_entities(cosmos world) override;
		void perform_logic_step(cosmos world) override;
		void execute_drawcalls_for_camera(viewing_step&) override;
	};
}