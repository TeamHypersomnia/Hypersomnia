#pragma once
#include "entity_system/processing_system.h"

#include "../components/ai_component.h"
#include "../components/transform_component.h"

using namespace augmentations;
using namespace entity_system;

class ai_system : public processing_system_templated<components::transform, components::ai> {
public:
	ai_system();

	void process_entities(world&) override;

	int draw_triangle_edges;
	int draw_cast_rays;
	int draw_discontinuities;
	int draw_memorised_walls;

	float epsilon_ray_angle_variation;
	float epsilon_distance_vertex_hit;
	float epsilon_threshold_obstacle_hit;
	float epsilon_max_segment_difference;
};