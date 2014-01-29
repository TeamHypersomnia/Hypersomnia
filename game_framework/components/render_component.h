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

		bool flip_horizontally;
		bool flip_vertically;

		render(resources::renderable* model = nullptr) : model(model), layer(0), mask(mask_type::WORLD), flip_horizontally(false), flip_vertically(false) {}
	};
}