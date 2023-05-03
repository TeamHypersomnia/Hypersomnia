#include "application/main/miniature_generator.h"
#include "augs/log.h"

augs::image combine_grid_into_whole(
	std::vector<augs::image>& imgs,
	unsigned columns,
	const bool flip_source_y,
	const vec2u target_size
) {
	if (imgs.empty()) {
		return augs::image();
	}

	auto do_blit = [&](auto& into, const auto& source_image, const auto dst) {
		const auto into_size = into.get_size();

		if (dst.x >= into_size.x) {
			return;
		}

		if (dst.y >= into_size.y) {
			return;
		}

		const auto source_size = source_image.get_size();
		const auto safe_source_size = vec2u(
			std::min(into_size.x - dst.x, source_size.x),
			std::min(into_size.y - dst.y, source_size.y)
		);

		if (flip_source_y) {
			for (auto y = 0u; y < safe_source_size.y; ++y) {
				for (auto x = 0u; x < safe_source_size.x; ++x) {
					into.pixel(dst + vec2u{ x, y }) = source_image.pixel(vec2u{ x, source_size.y - 1 - y });
				}
			}
		}
		else {
			for (auto y = 0u; y < safe_source_size.y; ++y) {
				for (auto x = 0u; x < safe_source_size.x; ++x) {
					into.pixel(dst + vec2u{ x, y }) = source_image.pixel(vec2u{ x, y });
				}
			}
		}
	};

	const auto rows = imgs.size() / columns;

	const auto cell_area = imgs[0].get_size();
	const auto whole_area = target_size;

	auto whole_image = augs::image(whole_area);

	for (unsigned x = 0; x < columns; ++x) {
		for (unsigned y = 0; y < rows; ++y) {
			const auto& source_image = imgs[y * columns + x];

			do_blit(
				whole_image,
				source_image,
				cell_area * vec2u(x, y)
			);
		}
	}

	return whole_image;
}

miniature_generator_state::capture_info miniature_generator_state::calc_info() const {
	const auto world_region_size = world_captured_region.get_size();

	const auto screen_space_region_size = world_region_size * zoom;
	const auto world_cell_size = screen_size / zoom;

	const auto grid_size = vec2u(
		std::ceil(screen_space_region_size.x / screen_size.x),
		std::ceil(screen_space_region_size.y / screen_size.y)
	);

	capture_info out;

	{
		const auto current_idx = screenshot_parts.size();

		const auto current_grid_coords = vec2u(
			current_idx % grid_size.x,
			current_idx / grid_size.x
		);

		const auto world_lt = world_captured_region.left_top();
		const auto half_offset = world_cell_size / 2;

		const auto world_current_cell_center = 
			world_lt
			+ current_grid_coords * world_cell_size
			+ half_offset
		;

		out.current_eye.transform.pos = world_current_cell_center;
		out.current_eye.zoom = zoom;

		out.current_screenshot_bounds = xywhi(0, 0, screen_size.x, screen_size.y);

		if (current_grid_coords.x == grid_size.x - 1) {
			out.current_screenshot_bounds.w = std::fmod(screen_space_region_size.x, screen_size.x);
		}

		if (current_grid_coords.y == grid_size.y - 1) {
			out.current_screenshot_bounds.h = std::fmod(screen_space_region_size.y, screen_size.y);
			out.current_screenshot_bounds.y = screen_size.y - out.current_screenshot_bounds.h;
		}
	}

	out.grid_size = grid_size;

	return out;
}

std::size_t miniature_generator_state::total_num_parts() const {
	return calc_info().grid_size.area();
}

augs::image miniature_generator_state::concatenate_parts() {
	return combine_grid_into_whole(screenshot_parts, calc_info().grid_size.x, true, vec2u(world_captured_region.get_size() * zoom));
}

void miniature_generator_state::save_to_file() {
	auto final_image = concatenate_parts();

	const auto unscaled_size = vec2(final_image.get_size());
	const auto bigger = unscaled_size.bigger_side();

	if (bigger > max_miniature_size) {
		const auto target_miniature_dimensions = unscaled_size * max_miniature_size / bigger;

		final_image.scale(vec2u(target_miniature_dimensions));
	}

	final_image.save_as_png(output_path);
}

camera_eye miniature_generator_state::get_current_camera() const {
	return calc_info().current_eye;
}

bool miniature_generator_state::complete() const {
	return screenshot_parts.size() == total_num_parts();
}

void miniature_generator_state::request_screenshot(augs::renderer& r) {
	if (request_in_progress) {
		return;
	}

	if (screenshot_parts.size() < total_num_parts()) {
		request_in_progress = true;
		const auto info = calc_info();
		const auto ss_bounds = info.current_screenshot_bounds;
		r.screenshot(ss_bounds);
	}
}

void miniature_generator_state::acquire(augs::image& img) {
	request_in_progress = false;
	screenshot_parts.emplace_back(std::move(img));
	//screenshot_parts.back().save_as_png(typesafe_sprintf("/tmp/ss/%x.png", screenshot_parts.size()));

	if (complete()) {
		save_to_file();
	}
}
