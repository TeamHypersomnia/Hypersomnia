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

		/* for script bindings */
		template<class derived>
		derived* get_renderable() {
			return reinterpret_cast<derived*>(model);
		}

		template<class derived>
		void set_renderable(derived* new_model) {
			model = new_model;
		}

		render(resources::renderable* model = nullptr) : model(model), layer(0), mask(mask_type::WORLD) {}
	};
}