#pragma once
#include "game/components/camera_component.h"
#include "game/components/transform_component.h"
#include "render_system.h"

using namespace augs;

class camera_system : public processing_system_templated<components::transform, components::camera> {
public:
	using processing_system_templated::processing_system_templated;

	void react_to_input_intents();

	void resolve_cameras_transforms_and_smoothing();
	void post_render_requests_for_all_cameras();
};