#pragma once
#include "game/messages/camera_render_request_message.h"

struct immediate_hud {
	void draw_circular_bars_and_nicknames(messages::camera_render_request_message);
};