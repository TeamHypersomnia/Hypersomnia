#include <filesystem>
#include "application/setups/server/tournament/tournament_state.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/filesystem/file.h"
#include "augs/misc/date_time.h"
#include "augs/log.h"

void tournament_state::save(const augs::path_type& path) const {
	try {
		augs::save_as_json_atomic(*this, path);
	}
	catch (const std::exception& err) {
		LOG("Failed to save tournament state to %x: %x", path.string(), err.what());
	}
}

namespace {
	/*
		Renames a state file the loader couldn't parse to a timestamped sibling
		instead of silently discarding it - the next successful save() would
		otherwise overwrite the original, losing forensic evidence of the corruption.
	*/
	void preserve_broken_state_file(const augs::path_type& path) {
		try {
			const auto stamp = augs::date_time().get_readable_for_file_long();
			auto broken_path = path;
			broken_path += std::string(".broken.") + stamp;

			std::error_code ec;
			std::filesystem::rename(path, broken_path, ec);

			if (ec) {
				LOG("Failed to preserve broken state file %x -> %x: %x", path.string(), broken_path.string(), ec.message());
			}
			else {
				LOG("Preserved unparseable state file as %x.", broken_path.string());
			}
		}
		catch (const std::exception& err) {
			LOG("Exception while preserving broken state file %x: %x", path.string(), err.what());
		}
	}
}

std::optional<tournament_state> tournament_state::load(const augs::path_type& path) {
	if (!augs::exists(path)) {
		return std::nullopt;
	}

	try {
		auto out = augs::from_json_file<tournament_state>(path);

		if (!out.initialized) {
			LOG("Tournament state file %x parsed but initialized=false; preserving and starting fresh.", path.string());
			::preserve_broken_state_file(path);
			return std::nullopt;
		}

		return out;
	}
	catch (const std::exception& err) {
		LOG("Failed to load tournament state from %x: %x", path.string(), err.what());
		::preserve_broken_state_file(path);
		return std::nullopt;
	}
}

std::vector<uint32_t> tournament_state::surviving_team_indices() const {
	std::vector<uint32_t> out;

	for (uint32_t i = 0; i < teams.size(); ++i) {
		if (!teams[i].eliminated) {
			out.push_back(i);
		}
	}

	return out;
}

bool tournament_state::tournament_finished() const {
	return surviving_team_indices().size() <= 1;
}

std::optional<uint32_t> tournament_state::sole_surviving_team_index() const {
	const auto alive = surviving_team_indices();

	if (alive.size() == 1) {
		return alive[0];
	}

	return std::nullopt;
}
