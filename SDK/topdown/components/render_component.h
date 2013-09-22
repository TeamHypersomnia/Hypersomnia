#pragma once
#include "entity_system/component.h"

namespace resources {
	struct render_info;
}

namespace components {
	struct render : public augmentations::entity_system::component {
		resources::render_info* info;

		/* for script bindings */
		template<class derived>
		derived* get_renderable() {
			return reinterpret_cast<derived*>(model);
		}

		template<class derived>
		void set_renderable(derived* new_model) {
			model = new_model;
		}

		render(resources::render_info* info) : info(info) {}
	};
}