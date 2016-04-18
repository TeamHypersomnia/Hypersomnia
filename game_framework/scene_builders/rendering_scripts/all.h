#pragma once
#include "game/components/camera_component.h"
#include "game/messages/camera_render_request_message.h"

namespace rendering_scripts {
	void standard_rendering(messages::camera_render_request_message msg);
}