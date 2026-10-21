#include "augs/filesystem/file.h"
#include "application/gui/arena_miniatures.h"
#include "application/setups/editor/project/editor_project_paths.h"

arena_miniatures& get_arena_miniatures() {
	static arena_miniatures miniatures;
	return miniatures;
}

static augs::path_type find_miniature_path(const std::string& arena_name) {
	const auto project_json = ::server_choose_arena_file_by(arena_name);

	if (project_json.empty()) {
		return {};
	}

	return augs::path_type(project_json).replace_filename(editor_project_paths::get_miniature_filename());
}

static std::optional<augs::file_time_type> find_write_time(const augs::path_type& path) {
	if (path.empty() || !augs::exists(path)) {
		return std::nullopt;
	}

	try {
		return augs::last_write_time(path);
	}
	catch (...) {
		return std::nullopt;
	}
}

const augs::atlas_entry* arena_miniatures::request(const std::string& arena_name) {
	if (arena_name.empty()) {
		return nullptr;
	}

	auto found = known.find(arena_name);

	if (found == known.end()) {
		auto arena = known_arena();
		arena.id = next_id++;
		arena.path = ::find_miniature_path(arena_name);
		arena.write_time = ::find_write_time(arena.path);

		if (arena.write_time.has_value()) {
			subjects_changed = true;
		}

		found = known.emplace(arena_name, std::move(arena)).first;
	}

	if (in_atlas == nullptr) {
		return nullptr;
	}

	return in_atlas->find(found->second.id);
}

void arena_miniatures::request_again_changed_on_disk() {
	for (auto& [arena_name, arena] : known) {
		auto path = ::find_miniature_path(arena_name);
		const auto write_time = ::find_write_time(path);

		if (const bool changed = path != arena.path || write_time != arena.write_time) {
			arena.path = std::move(path);
			arena.write_time = write_time;

			subjects_changed = true;
		}
	}
}

void arena_miniatures::set_in_atlas(const ad_hoc_in_atlas_map& new_in_atlas) {
	in_atlas = std::addressof(new_in_atlas);
}

std::optional<ad_hoc_atlas_subjects> arena_miniatures::take_new_subjects() {
	if (!subjects_changed) {
		return std::nullopt;
	}

	subjects_changed = false;

	auto subjects = ad_hoc_atlas_subjects();

	for (const auto& [arena_name, arena] : known) {
		if (arena.write_time.has_value()) {
			subjects.push_back({ arena.id, arena.path });
		}
	}

	return subjects;
}
