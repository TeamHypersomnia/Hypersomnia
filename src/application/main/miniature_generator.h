#pragma once
#include <cstddef>
#include "augs/filesystem/path_declaration.h"
#include "augs/math/camera_cone.h"
#include "augs/graphics/renderer.h"
#include "augs/image/image.h"

class miniature_generator_state {
	std::vector<augs::image> screenshot_parts;

	struct capture_info {
		vec2u grid_size;
		camera_eye current_eye;

		xywhi current_screenshot_bounds;
	};

	bool request_in_progress = false;

	capture_info calc_info() const;
	std::size_t total_num_parts() const;

	augs::image concatenate_parts();

	void save_to_file();

public:
	augs::path_type output_path;

	ltrb world_captured_region;
	float zoom = 1.f;

	vec2 screen_size;
	float max_miniature_size = 400.f;
	bool reveal_when_complete = false;

	camera_eye get_current_camera() const;
	bool complete() const;
	void request_screenshot(augs::renderer& r);
	void acquire(augs::image& img);
};

