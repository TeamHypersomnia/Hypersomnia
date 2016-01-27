#pragma once

#include "scene_builder.h"
#include "augs/entity_system/entity_id.h"

namespace scene_builders {
	struct one_entity : public scene_builder {
		void initialize(augs::world& world) override;
		void perform_logic_step(augs::world& world) override;
	};
}