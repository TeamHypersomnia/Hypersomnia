#pragma once
#include "entity_system/processing_system.h"

#include "../components/steering_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"
#include "../components/visibility_component.h"

using namespace augmentations;
using namespace entity_system;

class physics_system;

class steering_system : public processing_system_templated<components::transform, components::physics, components::steering> {
public:
	void substep(world&) override;
	void process_entities(world&) override;
	void process_events(world&) override;

	typedef std::vector < std::pair < augmentations::vec2<>, augmentations::vec2<>>> edges;

	struct steering_input {
		vec2<> position;
		vec2<> velocity;
		vec2<> unit_vel;
		vec2<> direction;
		vec2<> target;
		bool directed;

		void set_target(vec2<>);
		void set_velocity(vec2<>);

		float speed;
		float distance;
		float max_speed;
		float radius_of_effect;
		steering_input();
	};

	struct avoidance_input : steering_input {
		float avoidance_rectangle_length;
		float avoidance_rectangle_width;
		float ignore_discontinuities_narrower_than;
		std::vector<b2Vec2>* shape_verts;
		avoidance_input();
	};

	struct obstacle_avoidance_input : avoidance_input {
		edges* visibility_edges;
		std::vector<components::visibility::discontinuity>* discontinuities;

		vec2<>* output;

		obstacle_avoidance_input();
	};

	struct wander_input : steering_input {
		float circle_radius, circle_distance;
		float* current_angle;
		float displacement_degrees;
		wander_input();
	};

	struct containment_input : avoidance_input {
		int ray_count;
		bool randomize_rays;
		bool only_threats_in_OBB;
		physics_system* physics;
		b2Filter* ray_filter;
		entity* ignore_entity;
		containment_input();
	};

	struct avoidance_info_output {
		std::vector<int> intersections;
		vec2<> rightmost_line[2];
		b2Vec2 avoidance[4];
	} get_avoidance_info(obstacle_avoidance_input);


	bool avoid_collisions(obstacle_avoidance_input);
	vec2<> containment(containment_input);
	vec2<> seek(steering_input);
	vec2<> flee(steering_input);
	vec2<> wander(wander_input);

	vec2<> predict_interception(steering_input, vec2<> target_velocity, float max_prediction_ms, bool flee_prediction);
};