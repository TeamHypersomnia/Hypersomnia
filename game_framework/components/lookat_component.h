#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"

namespace components {
	struct lookat : public augs::entity_system::component {
		enum look_type {
			POSITION,
			VELOCITY,
			ACCELEARATION
		};

		enum lookat_easing {
			NONE,
			LINEAR,
			EXPONENTIAL
		};
		
		int easing_mode;
		
		/* for exponential smoothing */
		double smoothing_average_factor, averages_per_sec;
		
		/* for linear smoothing */
		augs::vec2<> last_rotation_interpolant;

		unsigned look_mode;

		augs::entity_system::entity_ptr target;

		lookat(augs::entity_system::entity* target = nullptr, unsigned look_mode = look_type::POSITION) 
			: target(target), look_mode(look_mode), smoothing_average_factor(0.5), averages_per_sec(20.0), easing_mode(NONE) {}
	};
}