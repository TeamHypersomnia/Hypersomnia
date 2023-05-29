#pragma once
#include "game/messages/message.h"
#include "game/messages/visibility_information.h"

struct pure_color_highlight_input {
	float maximum_duration_seconds = 0.f;
	float starting_alpha_ratio = 0.f;
	rgba color = white;
};

namespace messages {
	struct pure_color_highlight : message {
		pure_color_highlight_input input;
	};
}