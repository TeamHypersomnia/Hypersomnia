#pragma once

struct editor_player {
	bool show = false;
	bool paused = true;
	double speed = 1.0;

	auto get_speed() const {
		return paused ? 0.0 : speed;
	}

	/* A convenience synonym */
	bool is_editing_mode() const {
		return paused;
	}
};
