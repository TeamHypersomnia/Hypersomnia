#pragma once
#include "game/messages/camera_render_request_message.h"

struct immediate_hud {
	void draw_circular_bars(messages::camera_render_request_message);
	void draw_circular_bars_information(messages::camera_render_request_message);
};