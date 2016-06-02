#pragma once
#include "entity_system/processing_system.h"
#include "../components/trace_component.h"
#include "../components/render_component.h"
#include "../components/transform_component.h"

class trace_system : public augs::processing_system_templated<components::trace, components::render, components::transform> {
public:
	using processing_system_templated::processing_system_templated;

	void lengthen_sprites_of_traces();
	void destroy_outdated_traces();
	void spawn_finishing_traces_for_destroyed_objects();
};