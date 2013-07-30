#pragma once
#include "../../../entity_system/entity_system.h"

struct renderable;
namespace components {
	struct render : public augmentations::entity_system::component {
		unsigned layer;
		renderable* instance;

		enum {
			WORLD,
			GUI
		};

		unsigned mask;

		render(unsigned layer, renderable* instance, unsigned mask = WORLD)
			: layer(layer), instance(instance), mask(mask) {}
	};
}