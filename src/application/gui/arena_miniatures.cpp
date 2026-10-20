#include <algorithm>
#include "augs/math/rects.h"
#include "augs/filesystem/file.h"
#include "augs/log.h"
#include "application/gui/arena_miniatures.h"
#include "application/setups/editor/project/editor_project_paths.h"

arena_miniatures& get_arena_miniatures() {
	static arena_miniatures miniatures;
	return miniatures;
}

const augs::atlas_entry* arena_miniatures::request(const std::string& arena_name) {
	if (arena_name.empty()) {
		return nullptr;
	}

	if (const auto found = known.find(arena_name); found != known.end()) {
		if (found->second.has_value()) {
			return std::addressof(*found->second);
		}

		return nullptr;
	}

	if (std::find(pending.begin(), pending.end(), arena_name) == pending.end()) {
		pending.push_back(arena_name);
	}

	return nullptr;
}

void arena_miniatures::place(const std::string& arena_name) {
	const auto miniature_path = [&]() -> augs::path_type {
		const auto project_json = ::server_choose_arena_file_by(arena_name);

		if (project_json.empty()) {
			return {};
		}

		return augs::path_type(project_json).replace_filename(editor_project_paths::get_miniature_filename());
	}();

	auto& entry = known[arena_name];
	entry = std::nullopt;

	if (miniature_path.empty() || !augs::exists(miniature_path)) {
		return;
	}

	augs::image miniature;

	try {
		miniature.from_file(miniature_path);
	}
	catch (...) {
		LOG("Could not load the arena miniature: %x", miniature_path);
		return;
	}

	const auto original_size = miniature.get_size();

	if (original_size.x == 0 || original_size.y == 0) {
		return;
	}

	/*
		Fit into the cell keeping the aspect ratio.
	*/

	const auto fit_mult = std::min(
		static_cast<float>(cell_size_v) / original_size.x,
		static_cast<float>(cell_size_v) / original_size.y
	);

	const auto fitted_size = vec2u(
		std::clamp(static_cast<unsigned>(original_size.x * fit_mult), 1u, cell_size_v),
		std::clamp(static_cast<unsigned>(original_size.y * fit_mult), 1u, cell_size_v)
	);

	miniature.scale(fitted_size);

	const auto cell_index = used_cells++;
	const auto cell_lt = vec2u(cell_index % cells_per_row_v, cell_index / cells_per_row_v) * cell_size_v;

	for (unsigned y = 0; y < fitted_size.y; ++y) {
		for (unsigned x = 0; x < fitted_size.x; ++x) {
			grid.pixel(cell_lt + vec2u(x, y)) = miniature.pixel(vec2u(x, y));
		}
	}

	const auto grid_side = static_cast<float>(cell_size_v * cells_per_row_v);

	augs::atlas_entry placed;
	placed.atlas_space = xywh(vec2(cell_lt) / grid_side, vec2(fitted_size) / grid_side);
	placed.was_successfully_packed = true;
	placed.cached_original_size_pixels = fitted_size;

	entry = placed;
}

std::optional<augs::image> arena_miniatures::take_new_grid() {
	if (pending.empty()) {
		return std::nullopt;
	}

	/*
		At most one cell per newly asked arena - one without a miniature takes none.
	*/

	const auto max_new_cells = static_cast<unsigned>(pending.size());
	const auto now = std::chrono::steady_clock::now();

	const bool can_start_anew = 
		!when_started_anew.has_value() 
		|| now - *when_started_anew > std::chrono::seconds(3)
	;

	if (grid.get_size().area() == 0 || (used_cells + max_new_cells > max_cells_v && can_start_anew)) {
		grid.recreate_filled(vec2u::square(cell_size_v * cells_per_row_v));
		known.clear();
		used_cells = 0;
		when_started_anew = now;
	}

	auto newly_asked = std::move(pending);
	pending.clear();

	for (const auto& arena_name : newly_asked) {
		if (used_cells < max_cells_v) {
			place(arena_name);
		}
		else {
			known[arena_name] = std::nullopt;
		}
	}

	return grid;
}
