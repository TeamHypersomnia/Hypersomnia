#pragma once
#include "entity_system/processing_system.h"

#include "../components/physics_component.h"
#include "../components/visibility_component.h"
#include "../components/transform_component.h"

using namespace augs;

class visibility_system : public processing_system_templated<components::transform, components::visibility> {
public:
	using processing_system_templated::processing_system_templated;

	void generate_visibility_and_sight_information();

	bool draw_triangle_edges = true;
	bool draw_cast_rays = false;
	bool draw_discontinuities = false;
	bool draw_visible_walls = false;

	float epsilon_ray_distance_variation = 0.f;
	float epsilon_distance_vertex_hit = 0.f;
	float epsilon_threshold_obstacle_hit = 0.f;
};