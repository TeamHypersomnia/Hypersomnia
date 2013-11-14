#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"
#include "graphics/pixel.h"
#include "utility/timer.h"

#include "ai_component.h"

class steering_system;

namespace components {
	struct steering : public augmentations::entity_system::component {
		typedef augmentations::entity_system::entity_ptr target;
		
		struct behaviour {
			enum {
				SEEK, FLEE, ARRIVAL, PURSUIT, EVASION, OBSTACLE_AVOIDANCE, CONTAINMENT
			} behaviour_type;

			target current_target;

			float max_target_future_prediction_ms;
			float arrival_slowdown_radius;
			float effective_fleeing_radius;
			float max_force_applied;
			float weight;
		
			float intervention_time_ms;
			float avoidance_rectangle_width;

			float decision_duration_ms;

			bool erase_when_target_reached;

			bool randomize_rays;
			int ray_count;
			int visibility_type;

			bool enabled;

			augmentations::graphics::pixel_32 force_color;

			augmentations::util::timer last_decision_timer;
			augmentations::vec2<> last_decision;

			behaviour() : 
				weight(1.f), 
				erase_when_target_reached(true), 
				enabled(true), 
				behaviour_type(SEEK), 
				max_force_applied(-1.f), 
				max_target_future_prediction_ms(0.f),
				arrival_slowdown_radius(0.f),
				effective_fleeing_radius(-1.f),
				intervention_time_ms(0.f),
				avoidance_rectangle_width(0.f),
				decision_duration_ms(0.f),
				ray_count(0),
				randomize_rays(false),
				visibility_type(ai::visibility::OBSTACLE_AVOIDANCE)
			{
				last_decision_timer.reset();
			}

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