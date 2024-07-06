#include <future>
#include <mutex>
#include "application/gui/headless_map_catalogue.h"
#include "application/setups/client/arena_downloading_session.h"
#include "application/setups/client/https_file_downloader.h"

#include "3rdparty/include_httplib.h"
#include "application/gui/map_catalogue_gui.h"
#include "augs/misc/httplib_utils.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/filesystem/directory.h"
#include "augs/readwrite/byte_file.h"
#include "augs/string/path_sanitization.h"
#include "application/arena/arena_paths.h"
#include "augs/window_framework/window.h"
#include "augs/graphics/imgui_payload.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/misc/date_time.h"
#include "augs/templates/in_order_of.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "application/setups/editor/project/editor_project_meta.h"
#include "augs/readwrite/to_bytes.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "augs/string/path_sanitization.h"
#include "application/gui/headless_map_catalogue.hpp"
#include "augs/misc/async_get.h"

using S = map_catalogue_entry::state;

S map_catalogue_entry::get_state() const {
	if (version_on_disk.has_value()) {
		if (version_on_disk == version_timestamp) {
			return S::ON_DISK;
		}
		else {
			return S::UPDATE_AVAILABLE;
		}
	}

	return S::NOT_FOUND;
}

std::string sanitize_arena_short_description(std::string in) {
	int newlines = 0;

	for (auto& c : in) {
		if (c == '\n') {
			++newlines;

			if (newlines > 1) {
				c = ' ';
			}
		}
	}

	return in;
}

static void rescan_versions_on_disk(std::vector<map_catalogue_entry>& into) {
	const auto downloads_directory = augs::path_type(DOWNLOADED_ARENAS_DIR);

	for (auto& m : into) {
		try {
			const auto path = downloads_directory / m.name;

			m.version_on_disk = read_meta_from(path).version_timestamp;
		}
		catch (...) {
			m.version_on_disk = std::nullopt;
		}
	}
}

headless_map_catalogue::headless_map_catalogue() {
	rescan_official_arenas();
}

headless_map_catalogue::~headless_map_catalogue() = default;

void headless_map_catalogue::rescan_versions_on_disk() {
	::rescan_versions_on_disk(map_list);
}

bool headless_map_catalogue::finalize_download() {
	if (downloading.has_value() && downloading->finished()) {
		if (const auto error = downloading->get_error()) {
			auto popup = simple_popup();

			popup.title = "Failed to download maps";
			popup.message = *error;

			LOG_NOFORMAT(popup.make_log());

			download_failed_popup = popup;
		}

		downloading = std::nullopt;
		rescan_versions_on_disk();
		should_rescan = false;

		return true;
	}

	return false;
}

bool headless_map_catalogue::list_refresh_in_progress() const {
	return augs::in_progress(future_get) || future_response.valid();
}

void headless_map_catalogue::request_rescan() {
	should_rescan = true;
}

void headless_map_catalogue::request_refresh() {
	refreshed_once = false;
}

void headless_map_catalogue::rescan_official_arenas() {
	official_names.clear();

	auto register_arena = [&](const auto& path) {
		official_names.emplace(path.filename().string());
		return callback_result::CONTINUE;
	};

	const auto source_directory = augs::path_type(OFFICIAL_ARENAS_DIR);

	augs::for_each_directory_in_directory(source_directory, register_arena);
}

arena_synchronizer_input headless_map_catalogue::launch_download_all(const address_string_type& address) {
	arena_synchronizer_input downloadable_arenas;

	for (const auto& s : map_list) {
		if (s.get_state() != S::ON_DISK) {
			downloadable_arenas.push_back({ s.name, s.version_timestamp, s.last_displayed_index });
		}
	}

	sort_range(downloadable_arenas);

	launch_download(downloadable_arenas, parsed_url(address));

	return downloadable_arenas;
}

headless_catalogue_result headless_map_catalogue::advance(const headless_map_catalogue_input in) {
	auto result = headless_catalogue_result::IN_PROGRESS;

	if (!refreshed_once) {
		refreshed_once = true;
		refresh(in.external_arena_files_provider);
	}

	if (should_rescan) {
		should_rescan = false;

		if (!downloading.has_value() && !list_refresh_in_progress()) {
			LOG("Rescanning map catalogue on disk due to window activate.");
			rescan_versions_on_disk();
		}
	}

	if (downloading.has_value()) {
		downloading->advance();
	}

	if (augs::is_ready(future_get)) {
		future_response = launch_async(
			[response = augs::get_once(this->future_get), &last_error = this->list_catalogue_error, officials = this->official_names]() -> std::vector<map_catalogue_entry> {
				if (!httplib_utils::successful(response.status)) {
					last_error = typesafe_sprintf("Request failed with status: %x", response.status);
					return {};
				}

				try {
					auto result = augs::from_json_string<std::vector<map_catalogue_entry>>(response.body);

					erase_if(result, [&](const auto& e) { 
						const auto downloads_directory = augs::path_type(DOWNLOADED_ARENAS_DIR);
						const auto arena_name = e.name;
						const auto arena_folder_path = downloads_directory / arena_name;

						const auto sanitized = sanitization::sanitize_arena_path(downloads_directory, arena_name);
						const auto sanitized_path = std::get_if<augs::path_type>(&sanitized);

						if (!sanitized_path || *sanitized_path != arena_folder_path) {
							return true;
						}

						return found_in(officials, e.name); 
					});

					for (auto& r : result) {
						r.short_description = sanitize_arena_short_description(r.short_description);
					}

					::rescan_versions_on_disk(result);

					return result;
				}
				catch (const std::runtime_error& err) {
					last_error = err.what();
				}
				catch (...) {
					last_error = "Unknown error during deserialization.";
				}

				return {};
			}
		);
	}

	if (valid_and_is_ready(future_response)) {
		map_list = future_response.get();

		sort_range(
			map_list,

			[&](const auto& a, const auto& b) {
				return a.version_timestamp > b.version_timestamp;
			}
		);

		should_rescan = false;
		result = headless_catalogue_result::LIST_REFRESH_COMPLETE;
	}

	return result;
}

void headless_map_catalogue::refresh(const address_string_type address) {
	using namespace httplib;
	using namespace httplib_utils;

	if (downloading.has_value() || list_refresh_in_progress()) {
		return;
	}

	map_list.clear();
	list_catalogue_error = {};

	if (const auto parsed = parsed_url(address); parsed.valid()) {
		future_get = augs::async_get(
			parsed.get_base_url(),
			parsed.location + "?format=json"
		);
	}
	else {
		list_catalogue_error = typesafe_sprintf("Couldn't parse URL: %x", address);
	}

}

struct multi_arena_synchronizer_internal {
	https_file_downloader external;
	std::optional<arena_downloading_session> current_session;

	multi_arena_synchronizer_internal(const parsed_url& url) : external(url) {}

	auto make_requester(std::string arena_name) {
		return [&downloader = this->external, arena_name](const augs::secure_hash_type&, const augs::path_type& path) {
			const auto location = typesafe_sprintf("%x/%x", arena_name, path.string());
			downloader.download_file(location);
		};
	}
};

multi_arena_synchronizer::multi_arena_synchronizer(
	const arena_synchronizer_input& arena_names,
	const parsed_url& parent_folder_url
) : 
	input(arena_names), 
	data(std::make_unique<multi_arena_synchronizer_internal>(parent_folder_url))
{
	init_next_session();
}

multi_arena_synchronizer::~multi_arena_synchronizer() = default;

float multi_arena_synchronizer::get_current_file_percent_complete() const {
	if (data->external.get_current_total_bytes() == 0) {
		return 0.0f;
	}

	return float(data->external.get_current_downloaded_bytes()) / data->external.get_current_total_bytes();
}

void multi_arena_synchronizer::init_next_session() {
	if (current_map < input.size()) {
		const auto& current_input = input[current_map];

		data->current_session.emplace(
			current_input.name,
			current_input.version,
			data->make_requester(current_input.name)
		);
	}
}

arena_downloading_session& multi_arena_synchronizer::session() {
	return *data->current_session;
}

const arena_downloading_session& multi_arena_synchronizer::session() const {
	return *data->current_session;
}

void multi_arena_synchronizer::advance() {
	if (finished()) {
		return;
	}

	if (const auto new_file = data->external.get_downloaded_file()) {
		session().advance_with(augs::make_ptr_read_stream(new_file->second));
	}

	if (session().finished()) {
		if (session().has_error()) {
			LOG("Failed to download %x: %x", session().get_arena_name(), session().get_error());

			last_error = session().get_error();
		}

		++current_map;

		init_next_session();
	}
}

bool multi_arena_synchronizer::finished() const {
	return current_map >= input.size();
}

std::optional<std::string> multi_arena_synchronizer::get_error() const {
	return last_error;
}

std::optional<std::string> multi_arena_synchronizer::get_current_map_name() const {
	if (current_map < input.size()) {
		return input[current_map].name;
	}

	return std::nullopt;
}

float multi_arena_synchronizer::get_current_map_progress() const {
	if (data->current_session.has_value()) {
		return session().get_total_percent_complete(get_current_file_percent_complete());
	}

	return 1.0f;
}

float multi_arena_synchronizer::get_total_progress() const {
	std::size_t all = 0;
	float total = 0.0f;

	for_each_with_progress([&](const auto&, const float progress) {
		++all;
		total += progress;
	});

	if (all == 0) {
		return 1.0f;
	}

	return total / all;
}

