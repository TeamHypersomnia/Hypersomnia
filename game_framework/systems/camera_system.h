#pragma once
#include "../components/camera_component.h"
#include "../components/transform_component.h"
#include "render_system.h"

using namespace augs;

class camera_system : public processing_system_templated<components::transform, components::camera> {
	augs::misc::timer smooth_timer;
public:
	void consume_events(world&);
	void process_entities(world&);

	void process_rendering(world& owner);
};