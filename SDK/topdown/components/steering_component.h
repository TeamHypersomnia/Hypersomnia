#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"
#include "graphics/pixel.h"

class steering_system;

namespace components {
	struct steering : public augmentations::entity_system::component {
		typedef augmentations::entity_system::entity_ptr target;
		
		struct behaviour {
			enum {
				SEEK, FLEE, ARRIVAL, PURSUIT, EVASION
			} behaviour_type;

			target current_target;

			float max_target_future_prediction_ms;
			float arrival_slowdown_radius;
			float effective_fleeing_radius;
			float max_force_applied;
			float weight;
		
			bool erase_when_target_reached;
			bool enabled;

			augmentations::graphics::pixel_32 force_color;

			behaviour() : 
				weight(1.f), 
				erase_when_target_reached(true), 
				enabled(true), 
				behaviour_type(SEEK), 
				max_force_applied(-1.f), 
				max_target_future_prediction_ms(0.f),
				arrival_slowdown_radius(0.f),
				effective_fleeing_radius(-1.f)
			{}

			augmentations::vec2<> last_estimated_pursuit_position;
		private:
			friend class steering_system;
			bool completed;
		};

		std::vector<behaviour*> active_behaviours;
		float max_resultant_force;

		steering() : max_resultant_force(-1.f) {}

		/* binding facility */
		void add_behaviour(behaviour* b) { active_behaviours.push_back(b); }
		void clear_behaviours() { active_behaviours.clear(); }
	};
}