#pragma once
#include "entity_system/component.h"

namespace resources {
	struct renderable;
}

namespace components {
	struct render : public augs::entity_system::component {
		resources::renderable* model = nullptr;

		enum mask_type {
			WORLD,
			GUI
		};

		unsigned layer = 0;
		unsigned mask = mask_type::WORLD;

		bool flip_horizontally = false;
		bool flip_vertically = false;
		bool absolute_transform = false;

		template <typename T>
		T* get_renderable() {
			return (T*) model;
		}
	};
}