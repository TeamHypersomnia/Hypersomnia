#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"
#include "graphics/pixel.h"
#include "utility/timer.h"

#include "visibility_component.h"

class steering_system;

namespace components {
	struct steering : public augmentations::entity_system::component {
		typedef augmentations::entity_system::entity_ptr target;
		
		struct behaviour {
			enum {
				SEEK, FLEE, ARRIVAL, PURSUIT, EVASION, OBSTACLE_AVOIDANCE, CONTAINMENT, WANDER
			} behaviour_type;

			target current_target;

			float max_target_future_prediction_ms;
			float radius_of_effect;
			float max_force_applied;
			float weight;
		
			float intervention_time_ms;
			float max_intervention_length;
			float avoidance_rectangle_width;

			float decision_duration_ms;

			bool erase_when_target_reached;

			bool randomize_rays;
			bool only_threats_in_OBB;
			int ray_count;
			int visibility_type;

			float ignore_discontinuities_narrower_than;

			float wander_circle_radius;
			float wander_circle_distance;
			float wander_current_angle;

			float wander_displacement_degrees;

			augmentations::vec2<> last_output_force;

			bool enabled;

			augmentations::graphics::pixel_32 force_color;

			behaviour() : 
				weight(1.f), 
				erase_when_target_reached(true), 
				enabled(true), 
				behaviour_type(SEEK), 
				max_force_applied(-1.f), 
				max_target_future_prediction_ms(0.f),
				radius_of_effect(-1.f),
				intervention_time_ms(0.f),
				avoidance_rectangle_width(0.f),
				decision_duration_ms(0.f),
				ray_count(0),
				randomize_rays(false),
				only_threats_in_OBB(false),
				visibility_type(visibility::OBSTACLE_AVOIDANCE),
				ignore_discontinuities_narrower_than(1.f), max_intervention_length(-1.f),
				wander_circle_radius(0.f), wander_circle_distance(0.f),
				wander_displacement_degrees(0.f),
				wander_current_angle(0.f)
			{
			}

			augmentations::vec2<> last_estimated_pursuit_position;
		private:
			friend class steering_system;
		};

		std::vector<behaviour*> active_behaviours;
		float max_resultant_force;

		steering() : max_resultant_force(-1.f) {}

		/* binding facility */
		void add_behaviour(behaviour* b) { active_behaviours.push_back(b); }
		void clear_behaviours() { active_behaviours.clear(); }
	};
}