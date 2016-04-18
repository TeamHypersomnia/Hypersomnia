#pragma once
#include "game_framework/components/camera_component.h"
#include "game_framework/messages/camera_render_request_message.h"

namespace rendering_scripts {
	void standard_rendering(messages::camera_render_request_message msg);
}