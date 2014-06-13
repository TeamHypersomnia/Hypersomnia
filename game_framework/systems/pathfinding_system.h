#pragma once
#include "entity_system/processing_system.h"

#include "../components/pathfinding_component.h"
#include "../components/visibility_component.h"
#include "../components/transform_component.h"
#include "../components/physics_component.h"

using namespace augs;
using namespace entity_system;

class pathfinding_system : public processing_system_templated<components::transform, components::visibility, components::pathfinding, components::physics> {
public:
	pathfinding_system();

	void process_entities(world&);
	float epsilon_max_segment_difference;
	float epsilon_distance_visible_point;
	float epsilon_distance_the_same_vertex;

	int draw_memorised_walls;
	int draw_undiscovered;
};