#pragma once
#include "entity_system/component.h"

struct renderable;
struct sprite;
namespace components {
	struct render : public augmentations::entity_system::component {
		unsigned layer;
		renderable* instance;

		/* for script bindings */
		template<class derived>
		derived* get_renderable() {
			return reinterpret_cast<derived*>(instance);
		}

		template<class derived>
		void set_renderable(derived* new_instance) {
			instance = new_instance;
		}

		enum mask_type {
			WORLD,
			GUI
		};

		unsigned mask;

		render(unsigned layer = 0, renderable* instance = nullptr, unsigned mask = WORLD)
			: layer(layer), instance(instance), mask(mask) {}
	};
}