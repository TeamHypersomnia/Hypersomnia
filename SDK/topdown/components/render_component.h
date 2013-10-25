#pragma once
#include "entity_system/component.h"

namespace resources {
	struct renderable;
}

namespace components {
	struct render : public augmentations::entity_system::component {
		resources::renderable* model;

		enum mask_type {
			WORLD,
			GUI
		};

		unsigned layer;
		unsigned mask;

		render(resources::renderable* model = nullptr) : model(model), layer(0), mask(mask_type::WORLD) {}
	};
}