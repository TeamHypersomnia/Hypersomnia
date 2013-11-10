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
};