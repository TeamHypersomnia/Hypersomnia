#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"
#include "graphics/pixel.h"
#include "misc/timer.h"

#include "visibility_component.h"

class steering_system;
class physics_system;

namespace components {
	struct visibility;

	struct steering : public augs::entity_system::component {
		struct behaviour_state;

		struct object_info {
			augs::vec2<> position, velocity, unit_vel;
			float speed, max_speed;

			void set_velocity(augs::vec2<>);
			object_info();
		};

		struct scene {
			object_info subject; 
			behaviour_state* state;
			physics_system* physics;
			visibility* vision;
			std::vector<b2Vec2>* shape_verts;

			augs::entity_system::entity* subject_entity;
			
			scene();
		};

		struct target_info {
			object_info info;

			augs::vec2<> direction;
			float distance;

			bool is_set;

			target_info();

			void set(const augs::entity_system::entity_ptr&);
			void set(augs::vec2<> position, augs::vec2<> velocity = augs::vec2<>(0.f, 0.f));

			void calc_direction_distance(const object_info& subject);
		};

		struct behaviour {
			augs::graphics::pixel_32 force_color;
			float max_force_applied;
			float weight;

			behaviour();
			virtual augs::vec2<> steer(scene) { return augs::vec2<>(); }
		};

		struct directed : behaviour {
			float radius_of_effect;
			float max_target_future_prediction_ms;

			directed();

			augs::vec2<> predict_interception(const object_info& subject, const target_info& target, bool flee_prediction);
			virtual augs::vec2<> steer(scene) { return augs::vec2<>(); }
		};

		struct avoidance : behaviour {
			float intervention_time_ms;
			float max_intervention_length;
			float avoidance_rectangle_width;

			avoidance();
			
			struct avoidance_info_output {
				augs::vec2<> rightmost_line[2];
				b2Vec2 avoidance[4];
			} get_avoidance_info(const scene&);

			std::vector<int> check_for_intersections(avoidance_info_output input, const std::vector<visibility::edge>& visibility_edges);

			void optional_align(scene& in);
			float get_avoidance_length(const object_info& subject) const;
			virtual augs::vec2<> steer(scene) { return augs::vec2<>(); }
		};

		/* now the actual implementations */

		struct seek : directed {
			augs::vec2<> seek_to(const object_info& subject, const target_info& target) const;

			virtual augs::vec2<> steer(scene) override;
		};

		struct flee : directed {
			augs::vec2<> flee_from(const object_info& subject, const target_info& target) const;

			virtual augs::vec2<> steer(scene) override;
		};

		struct wander : behaviour {
			float circle_radius;
			float circle_distance;

			float displacement_degrees;

			wander();
			virtual augs::vec2<> steer(scene) override;
		};

		struct containment : avoidance {
			bool randomize_rays;
			bool only_threats_in_OBB;
			int ray_count;

			b2Filter ray_filter;

			containment();
			virtual augs::vec2<> steer(scene) override;
		};
		
		struct flocking : behaviour {
			b2Filter group;
			/* works as "radius" */
			float square_side;
			float field_of_vision_degrees;

			flocking();
			virtual augs::vec2<> steer(scene) { return augs::vec2<>(); }
		};

		struct separation : flocking {
			virtual augs::vec2<> steer(scene) override;
		};

		struct obstacle_avoidance : avoidance {
			containment* navigation_correction;
			seek* navigation_seek;

			int visibility_type;
			float ignore_discontinuities_narrower_than;

			obstacle_avoidance();
			virtual augs::vec2<> steer(scene) override;
		};

		struct behaviour_state {
			/* subject behaviour info */
			behaviour* subject_behaviour;

			/*
			if this is not null for avoidance behaviours,
			these won't align the avoidance rectangle with unit velocity 
			but with direction to this entity, still using speed and and intervention time to calculate avoidance length
			*/

			target_info target;
			augs::entity_system::entity_ptr target_from;

			augs::vec2<> last_output_force;
			augs::vec2<> last_estimated_target_position;
			float weight_multiplier;

			bool enabled;

			float current_wander_angle;

			void update_target_info(const object_info& subject);
			behaviour_state(behaviour* subject_behaviour = nullptr)
				: subject_behaviour(subject_behaviour), current_wander_angle(0.f), enabled(true), weight_multiplier(1.f) {}
		};

		/* the only reason we are storing pointers instead of values are scripts */
		std::vector<behaviour_state*> active_behaviours;
		float max_resultant_force;

		steering() : max_resultant_force(-1.f) {}

		/* binding facility */
		void add_behaviour(behaviour_state* b) { active_behaviours.push_back(b); }
		void clear_behaviours() { active_behaviours.clear(); }

	};
}