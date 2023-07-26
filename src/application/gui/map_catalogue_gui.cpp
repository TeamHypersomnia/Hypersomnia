#include <future>
#include <mutex>
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
#include "augs/misc/time_utils.h"
#include "augs/templates/in_order_of.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "application/setups/editor/project/editor_project_meta.h"
#include "augs/readwrite/to_bytes.h"

constexpr auto miniature_size_v = 80;
constexpr auto preview_size_v = 400;

editor_project_meta read_meta_from(const augs::path_type& arena_folder_path);

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

void headless_map_catalogue::rescan_official_arenas() {
	official_names.clear();

	auto register_arena = [&](const auto& path) {
		official_names.emplace(path.filename().string());
		return callback_result::CONTINUE;
	};

	const auto source_directory = augs::path_type(OFFICIAL_ARENAS_DIR);

	augs::for_each_directory_in_directory(source_directory, register_arena);
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

template <class F>
void multi_arena_synchronizer::for_each_with_progress(F callback) const {
	/* 
		For now there's no parallel downloads,
		so all maps before the current have 100%, 0% after, and non-0 for the current.
	*/

	for (std::size_t i = 0; i < input.size(); ++i) {
		float progress = 0.0f;

		if (i < current_map) {
			progress = 1.0f;
		}
		else if (i == current_map) {
			progress = session().get_total_percent_complete(get_current_file_percent_complete());
		}

		callback(input[i].name, progress);
	}
}

float multi_arena_synchronizer::get_current_file_percent_complete() const {
	if (data->external.get_total_bytes() == 0) {
		return 0.0f;
	}

	return float(data->external.get_downloaded_bytes()) / data->external.get_total_bytes();
}

void multi_arena_synchronizer::init_next_session() {
	if (current_map < input.size()) {
		const auto& current_input = input[current_map];

		data->current_session = arena_downloading_session(
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

headless_map_catalogue::headless_map_catalogue() {
	rescan_official_arenas();
}

headless_map_catalogue::~headless_map_catalogue() = default;

map_catalogue_gui_state::map_catalogue_gui_state(const std::string& title) : base(title) {}
map_catalogue_gui_state::~map_catalogue_gui_state() = default;

std::mutex miniature_mutex;

void map_catalogue_gui_state::request_miniatures(const map_catalogue_input in) {
	using namespace httplib_utils;

	completed_miniatures.clear();
	completed_miniatures.resize(headless.get_map_list().size());

	has_next_miniature.store(0);

	future_downloaded_miniatures = launch_async([
		&miniature_counter = this->has_next_miniature,
		&result = this->completed_miniatures,
		&for_maps = this->headless.get_map_list(),
		address = in.external_arena_files_provider
	]() {
			if (const auto parsed = parsed_url(address); parsed.valid()) {
				const auto ca_path = CA_CERT_PATH;
				auto client = http_client_type(parsed.host);

#if BUILD_OPENSSL
				client.set_ca_cert_path(ca_path.c_str());
				client.enable_server_certificate_verification(true);
#endif
				client.set_follow_location(true);
				client.set_read_timeout(3);
				client.set_write_timeout(3);
				client.set_keep_alive(true);

				const auto miniatures_directory = augs::path_type(GENERATED_FILES_DIR) / "miniatures";

				augs::create_directories(miniatures_directory);

				auto download_miniature = [&](const auto& m) {
					/* 0 marks no miniature */
					const auto index = index_in(for_maps, m);
					const auto this_id = static_cast<ad_hoc_entry_id>(1 + index);

					const auto miniature_filename = typesafe_sprintf("%x.png", this_id);

					const auto next_path = miniatures_directory / miniature_filename;

					auto set_result = [&](const ad_hoc_entry_id id) {
						std::unique_lock<std::mutex> lock(miniature_mutex);
						result[index] = { id, next_path };
						LOG_NVPS(index, for_maps.size());
						++miniature_counter;
					};

					const auto location = typesafe_sprintf("%x/%x/miniature.png", parsed.location, m.name);

					auto response = client.Get(location.c_str());

					if (response == nullptr) {
						set_result(0);
						return;
					}

					if (!successful(response->status)) {
						set_result(0);
						return;
					}

					try {
						augs::save_string_as_bytes(response->body, next_path);
						set_result(this_id);
					}
					catch (...) {
						set_result(0);
					}
				};

				::in_order_of(
					for_maps,
					[&](const auto& entry) {
						return entry.version_timestamp;
					},
					[&](const auto& a, const auto& b) {
						return a > b;
					},
					download_miniature
				);
			}
		}
	);
}

void map_catalogue_gui_state::rebuild_miniatures() {
	mark_rebuild_miniatures = true;
}

std::optional<std::vector<ad_hoc_atlas_subject>> map_catalogue_gui_state::get_new_ad_hoc_images() {
	if (mark_rebuild_miniatures) {
		mark_rebuild_miniatures = false;
		return last_miniatures;
	}

	if (has_next_miniature > 0) {
		--has_next_miniature;

		{
			std::unique_lock<std::mutex> lock(miniature_mutex);
			last_miniatures = completed_miniatures;
		}

		const auto& map_list = headless.get_map_list();

		if (last_miniatures.size() != map_list.size()) {
			LOG_NVPS(last_miniatures.size(), map_list.size());
		}

		ensure(last_miniatures.size() == map_list.size());

		for (std::size_t i = 0; i < map_list.size(); ++i) {
			if (i < last_miniatures.size()) {
				map_list[i].miniature_id = last_miniatures[i].id;
			}
			else {
				map_list[i].miniature_id = 0;
			}
		}

		erase_if(last_miniatures, [](const auto& e) { return e.id == 0; });

		return last_miniatures;
	}

	return std::nullopt;
}

void headless_map_catalogue::rescan_versions_on_disk() {
	::rescan_versions_on_disk(map_list);
}

std::string sanitize_arena_short_description(std::string in);

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

void map_catalogue_gui_state::perform_list(const map_catalogue_input in) {
	using namespace augs::imgui;

	const auto num_columns = 2;

	int current_col = 0;

	auto do_column = [&](std::string label, std::optional<rgba> col = std::nullopt) {
		const bool is_current = current_col == sort_by_column;

		if (is_current) {
			label += ascending ? "▲" : "▼";
		}

		const auto& style = ImGui::GetStyle();

		auto final_color = rgba(is_current ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled]);

		if (col.has_value()) {
			final_color = *col;
		}

		auto col_scope = scoped_style_color(ImGuiCol_Text, final_color);

		if (ImGui::Selectable(label.c_str())) {
			if (is_current) {
				ascending = !ascending;
			}
			else {
				sort_by_column = current_col;
				ascending = true;
			}
		}

		++current_col;

		if (num_columns > 1) {
			ImGui::NextColumn();
		}
	};

	const auto avail = ImGui::GetContentRegionAvail();

	ImGui::Columns(num_columns);

	if (num_columns > 1) {
		ImGui::SetColumnWidth(0, avail.x * 0.8f);
	}

	do_column("Name");
	do_column("Last modified");

	ImGui::Separator();

	const auto line_h = ImGui::GetTextLineHeight();

	const auto downloads_directory = augs::path_type(DOWNLOADED_ARENAS_DIR);

	std::unordered_map<std::string, float> progress_of;

	{
		const auto& downloading = headless.get_downloading();

		if (downloading.has_value()) {
			downloading->for_each_with_progress(
				[&](const auto& name, const float progress) {
					progress_of[name] = progress;
				}
			);
		}
	}

	std::size_t display_idx = 0;

	auto process_entry = [&](const auto& entry) {
		entry.last_displayed_index = display_idx++;

		const auto arena_name = entry.name;
		const auto arena_folder_path = downloads_directory / arena_name;

		if (filter.IsActive() && !filter.PassFilter(arena_name.c_str()) && !filter.PassFilter(entry.author.c_str())) {
			entry.passed_filter = false;
			return;
		}
		else {
			entry.passed_filter = true;
		}

		const auto selectable_size = ImVec2(0, 1 + miniature_size_v);
		auto id = scoped_id(arena_name.c_str());

		auto entry_id = std::addressof(entry);
		const bool is_selected = found_in(selected_arenas, entry_id);
		auto maybe_progress = mapped_or_nullptr(progress_of, arena_name);

		if (maybe_progress && *maybe_progress >= 1.0f) {
			if (entry.version_timestamp != entry.version_on_disk) {
				selected_arenas.erase(entry_id);
			}

			entry.version_on_disk = entry.version_timestamp;
			maybe_progress = nullptr;
		}

		const bool downloading_this_map = maybe_progress != nullptr;

		const auto local_pos = ImGui::GetCursorPos();

		{
			auto selectable_cols = [&]() -> std::array<rgba, 3> {
				switch (entry.get_state()) {
					case S::ON_DISK:
						return {
							rgba(0, 46, 0, 131),
							rgba(0, 130, 0, 100),
							rgba(0, 130, 0, 150)
						};
					case S::UPDATE_AVAILABLE:
						return {
							rgba(46, 46, 0, 100),
							rgba(130, 130, 0, 100),
							rgba(130, 130, 0, 140)
						};

					case S::NOT_FOUND:
					default:
						return {
							rgba(255, 255, 255, 0),
							rgba(255, 255, 255, 30),
							rgba(255, 255, 255, 60)
						};
				}
			}();

			if (is_selected) {
				selectable_cols[0] = selectable_cols[1] = selectable_cols[2];
			}

			if (maybe_progress) {
				const auto downloaded_bg_color = rgba(12, 25, 60, 255);
				const auto fill_color = rgba(0, 130, 0, 255);

				if (auto window = ImGui::GetCurrentWindow()) {
					const auto min_x = window->ParentWorkRect.Min.x;
					const auto max_x = window->ParentWorkRect.Max.x;
					const auto avail_x = max_x - min_x;

					const auto flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_Disabled;

					{
						auto cscope = scoped_preserve_cursor();

						auto progress_bg_cols = scoped_selectable_colors({ downloaded_bg_color, downloaded_bg_color, downloaded_bg_color });

						auto fill_size = selectable_size;

						ImGui::Selectable("##EntryBgBg", true, flags, fill_size);
					}

					if (*maybe_progress > 0.0f) {
						auto cscope = scoped_preserve_cursor();

						auto progress_cols = scoped_selectable_colors({ fill_color, fill_color, fill_color });

						auto fill_size = selectable_size;
						fill_size.x = avail_x * (*maybe_progress);

						ImGui::Selectable("##EntryBg", true, flags, fill_size);
					}
				}
			}

			auto darkened_selectables = scoped_selectable_colors(selectable_cols);

			ImGui::Selectable("##Entry", !downloading_this_map, ImGuiSelectableFlags_SpanAllColumns, selectable_size);

			auto do_host = [&]() {
				open_host_server_window = arena_name;
			};

			auto do_update = [&]() {
				launch_download_on_last_selected = true;
			};

			auto select_this = [&]() {
				selected_arenas.clear();
				selected_arenas.emplace(entry_id);
				last_selected = entry_id;
			};

			if (ImGui::IsItemClicked()) {
				if (ImGui::GetIO().KeyCtrl) {
					if (found_in(selected_arenas, entry_id)) {
						selected_arenas.erase(entry_id);
						last_selected = nullptr;
					}
					else {
						selected_arenas.emplace(entry_id);
						last_selected = entry_id;
					}
				}
				else {
					select_this();
				}

				if (!ImGui::GetIO().KeyCtrl) {
					if (ImGui::IsMouseDoubleClicked(0)) {
						if (entry.get_state() == S::ON_DISK) {
							do_host();
						}
						else {
							do_update();
						}
					}
				}
			}

			if (ImGui::BeginPopupContextItem()) {
				if (!is_selected) {
					select_this();
				}

				{
					if (entry.get_state() == S::ON_DISK) {
						if (ImGui::Selectable("Host")) {
							do_host();
						}
					}
					else if (entry.get_state() == S::UPDATE_AVAILABLE) {
						if (ImGui::Selectable("Update")) {
							do_update();
						}
					}
					else if (entry.get_state() == S::NOT_FOUND) {
						if (ImGui::Selectable("Download")) {
							do_update();
						}
					}

					ImGui::Separator();

					auto scope = maybe_disabled_cols({}, entry.get_state() == S::NOT_FOUND);

					if (ImGui::Selectable("Reveal in explorer")) {
						in.window.reveal_in_explorer(arena_folder_path);
					}
				}

				ImGui::EndPopup();
			}
		}

		const auto after_pos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(local_pos);

		ImGui::SameLine();

		const auto image_padding = vec2(0, 0);
		const auto miniature_entry = mapped_or_nullptr(in.ad_hoc_atlas, entry.miniature_id);
		const auto target_miniature_size = vec2::square(miniature_size_v);

		if (entry.miniature_id != 0 && miniature_entry != nullptr) {
			const auto miniature_size = miniature_entry->get_original_size();
			const auto resized_miniature_size = vec2i::scaled_to_max_size(miniature_size, miniature_size_v);

			const auto offset = (target_miniature_size - resized_miniature_size) / 2;
			game_image(*miniature_entry, resized_miniature_size, white, offset + image_padding, augs::imgui_atlas_type::AD_HOC);
		}

		invisible_button("invisible_miniature", target_miniature_size + image_padding);

		ImGui::SameLine();

		const auto x = ImGui::GetCursorPosX();
		const auto prev_y = ImGui::GetCursorPosY() + line_h;

		text(arena_name);
		ImGui::SameLine();
		text_disabled("- by " + entry.author);

		ImGui::SetCursorPosY(prev_y);
		ImGui::SetCursorPosX(x);

		text_disabled(entry.short_description);

		ImGui::SetCursorPos(after_pos);

		if (num_columns > 1) {
			ImGui::NextColumn();

			if (maybe_progress) {
				if ((*maybe_progress) == 0.0f) {
					text_color(typesafe_sprintf("Queued."), cyan);
				}
				else {
					text_color(typesafe_sprintf("Downloading..."), cyan);
					text_color(typesafe_sprintf("%2f%", (*maybe_progress) * 100), cyan);
				}
			}
			else {
				const auto date = augs::date_time::from_utc_timestamp(entry.version_timestamp);

				text_disabled(date.how_long_ago_tell_seconds());
				tooltip_on_hover(date.get_readable_day_hour());

				const auto map_state = entry.get_state();

				if (map_state == S::ON_DISK) {
					text_color("Up to date.", green);
				}
				else if (map_state == S::UPDATE_AVAILABLE) {
					text_color("Update available!", yellow);
				}
			}

			ImGui::NextColumn();
		}
	};

	if (sort_by_column == 0) {
		::in_order_of(
			headless.get_map_list(),
			[&](const auto& entry) {
				return entry.name;
			},
			[&](const auto& a, const auto& b) {
				return ascending ? augs::natural_order(a, b) : augs::natural_order(b, a);
			},
			process_entry
		);
	}
	else {
		::in_order_of(
			headless.get_map_list(),
			[&](const auto& entry) {
				return entry.version_timestamp;
			},
			[&](const auto& a, const auto& b) {
				return ascending ? a < b : a > b;
			},
			process_entry
		);
	}

	ImGui::Columns(1);
}


void headless_map_catalogue::request_rescan() {
	should_rescan = true;
}

void map_catalogue_gui_state::request_rescan() {
	headless.request_rescan();
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

	if (valid_and_is_ready(future_response)) {
		map_list = future_response.get();

		should_rescan = false;
		result = headless_catalogue_result::LIST_REFRESH_COMPLETE;
	}

	return result;
}

bool map_catalogue_gui_state::perform(const map_catalogue_input in) {
	using namespace httplib_utils;
	using namespace augs::imgui;

	if (!show) {
		return false;
	}

	centered_size_mult = vec2(0.9f, 0.8f);

	auto imgui_window = make_scoped_window(ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	if (!imgui_window) {
		return false;
	}

	{
		const auto advance_result = headless.advance(in.make_headless());

		if (advance_result == headless_catalogue_result::LIST_REFRESH_COMPLETE) {
			focus_on_filter_once = true;

			request_miniatures(in);
		}
	}

	if (valid_and_is_ready(future_downloaded_miniatures)) {
		future_downloaded_miniatures.get();
	}

	bool address_modified = false;

	{
		auto scope = maybe_disabled_cols({}, refresh_in_progress() || is_downloading());

		if (input_text("Provider", in.external_arena_files_provider)) {

		}

		if (ImGui::IsItemDeactivatedAfterEdit()) {
			address_modified = true;
			refresh(in.external_arena_files_provider);
		}
	}

	{
		ImGui::SameLine();

		auto scope = maybe_disabled_cols({}, refresh_in_progress() || is_downloading());

		if (ImGui::Button("Refresh")) {
			refresh(in.external_arena_files_provider);
		}
	}

	if (headless.list_refresh_in_progress()) {
		text_color("Downloading catalogue...", yellow);
	}
	else {
		const auto last_error = headless.get_list_catalogue_error();

		if (last_error.size() > 0) {
			text_color("Failed to download the catalogue.\n", red);
			text_color(last_error, red);
		}
		else {
			if (focus_on_filter_once) {
				focus_on_filter_once = false;
				ImGui::SetKeyboardFocusHere(0);
			}

			if (ImGui::Button("Select all")) {
				bool all_selected_already = true;

				for (auto& entry : headless.get_map_list()) {
					if (entry.passed_filter) {
						const auto entry_id = std::addressof(entry);
						const bool is_selected = found_in(selected_arenas, entry_id);

						if (!is_selected) {
							all_selected_already = false;

							break;
						}
					}
				}

				for (auto& entry : headless.get_map_list()) {
					const auto entry_id = std::addressof(entry);

					if (entry.passed_filter) {
						if (all_selected_already) {
							selected_arenas.erase(entry_id);
						}
						else {
							selected_arenas.emplace(entry_id);
						}
					}
				}
			}

			ImGui::SameLine();
			filter_with_hint(filter, "##HierarchyFilter", "Filter maps...");

			const auto avail = ImGui::GetContentRegionAvail();
			const auto proj_list_width = avail.x * 0.6f;
			const auto proj_desc_width = avail.x * 0.4f;
			(void)proj_desc_width;

			const auto button_size_mult = 3.0f;
			const auto button_padding_mult = 0.2f;
			const auto text_h = ImGui::GetTextLineHeight();
			const auto space_for_dl_button = text_h * (button_padding_mult * 2 + button_size_mult);

			{
				auto scope = scoped_child("Project list view", ImVec2(proj_list_width, -space_for_dl_button));

				perform_list(in);
			}

			{
				ImGui::SameLine();

				auto fix_background_color = scoped_style_color(ImGuiCol_ChildBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});

				auto scope = scoped_child("Project description view", ImVec2(proj_desc_width, -space_for_dl_button), false);

				auto selected_entry = last_selected;

				if (selected_entry != nullptr) {
					auto& entry = *selected_entry;

					const auto image_padding = vec2(5, 5);
					const auto image_internal_padding = vec2i(15, 15);
					const auto preview_entry = mapped_or_nullptr(in.ad_hoc_atlas, entry.miniature_id);
					const auto target_preview_size = vec2::square(preview_size_v);

					if (preview_entry != nullptr) {
						const auto preview_size = preview_entry->get_original_size();
						const auto resized_preview_size = vec2i(vec2(preview_size) * (static_cast<float>(preview_size_v) / preview_size.bigger_side()));

						const auto offset = (target_preview_size - resized_preview_size) / 2;

						auto bg_size = target_preview_size + image_internal_padding * 2;
						bg_size.x = ImGui::GetContentRegionAvail().x;

						game_image(in.necessary_images[assets::necessary_image_id::BLANK], bg_size, rgba(0, 0, 0, 100), image_padding, augs::imgui_atlas_type::GAME);
						game_image(*preview_entry, resized_preview_size, white, offset + image_padding + image_internal_padding, augs::imgui_atlas_type::AD_HOC);

						invisible_button("invisible_preview", target_preview_size + image_padding + image_internal_padding * 2);

						auto fix_background_color = scoped_style_color(ImGuiCol_ChildBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});

						shift_cursor(image_padding);

						auto scope = scoped_child("descview", ImVec2(0, 0), true);

						text(std::string(entry.name));
						ImGui::SameLine();
						text_disabled(std::string("- by ") + entry.author);

						ImGui::Separator();

						const auto map_state = entry.get_state();

						if (map_state == S::ON_DISK) {
							text_color("This map is on your disk.", green);
							ImGui::Separator();
						}
						else if (map_state == S::UPDATE_AVAILABLE) {
							text_color("This map can be updated.", yellow);
							ImGui::Separator();
						}

						ImGui::PushTextWrapPos();

						text_color(entry.short_description, rgba(210, 210, 210, 255));

						ImGui::PopTextWrapPos();
					}
				}
				else {
					text_disabled("(No project selected)");
				}
			}

			auto do_big_button = [&](auto label, auto icon, auto col, std::array<rgba, 3> bg_cols, auto after) {
				return selectable_with_icon(
					in.necessary_images[icon],
					label,
					button_size_mult,
					vec2(1.0f, button_padding_mult),
					col,
					bg_cols, 0.0f, true,
					after
				);
			};

			const auto icon = assets::necessary_image_id::DOWNLOAD_ICON;

			const auto& downloading = headless.get_downloading();

			if (downloading.has_value()) {
				auto cols = maybe_disabled_cols({}, true);

				const auto total_progress = downloading->get_total_progress();

				if (auto window = ImGui::GetCurrentWindow(); window && total_progress > 0.0f) {
					const auto min_x = window->ParentWorkRect.Min.x;
					const auto max_x = window->ParentWorkRect.Max.x;
					const auto avail_x = max_x - min_x;

					const auto fill_color = rgba(20, 70, 20, 255);

					const auto flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_Disabled;

					auto cscope = scoped_preserve_cursor();

					auto progress_cols = scoped_selectable_colors({ fill_color, fill_color, fill_color });

					const auto text_h = ImGui::GetTextLineHeight();
					const auto button_size = ImVec2(0, text_h * button_size_mult);

					auto fill_size = button_size;
					fill_size.x = avail_x * total_progress;

					shift_cursor(vec2(0, text_h * button_padding_mult));

					ImGui::Selectable("##ButtonBgDownloadProgress", true, flags, fill_size);
				}

				const auto num_maps = downloading->get_input().size();
				const auto num_label = num_maps == 1 ? downloading->get_input()[0].name : typesafe_sprintf("%x maps", num_maps);
				const auto progress_label = typesafe_sprintf("Downloading %x... %f2%", num_label, total_progress * 100);

				auto after_cb = [&]() {
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
						auto arenas = std::string();

						downloading->for_each_with_progress(
							[&](const auto& name, const float progress) {
								arenas += typesafe_sprintf("%x: %2f%\n", name, progress);
							}
						);

						if (arenas.size() > 0) {
							arenas.pop_back();
						}

						text_tooltip(arenas);
					}
				};

				do_big_button(progress_label, icon, rgba(100, 255, 100, 255), {
					rgba(25, 25, 25, 0),
					rgba(50, 50, 50, 0),
					rgba(50, 50, 50, 0)
				}, after_cb);
			}
			else {
				arena_synchronizer_input downloadable_arenas;
			   
				for (const auto s : selected_arenas) {
					if (s->get_state() != S::ON_DISK) {
						downloadable_arenas.push_back({ s->name, s->version_timestamp, s->last_displayed_index });
					}
				}

				sort_range(downloadable_arenas);

				if (selected_arenas.empty()) {
					auto cols = maybe_disabled_cols({}, true);

					do_big_button("No map selected.", icon, rgba(100, 100, 100, 255), {
						rgba(25, 25, 25, 255),
						rgba(50, 50, 50, 255),
						rgba(50, 50, 50, 255)
					}, [](){});
				}
				else if (downloadable_arenas.empty()) {
					auto cols = maybe_disabled_cols({}, true);

					do_big_button("All selected arenas are up to date.", icon, rgba(100, 100, 100, 255), {
						rgba(25, 25, 25, 255),
						rgba(50, 50, 50, 255),
						rgba(50, 50, 50, 255)
					}, [](){});
				}
				else {
					auto dl_label = typesafe_sprintf("Download %x maps", downloadable_arenas.size());

					if (downloadable_arenas.size() == 1) {
						dl_label = "Download " + (*downloadable_arenas.begin()).name;
					}

					auto after_cb = [&]() {
						if (ImGui::IsItemHovered()) {
							if (downloadable_arenas.size() > 1) {
								auto arenas = std::string();

								for (const auto& s : downloadable_arenas) {
									arenas += s.name + "\n";
								}

								if (arenas.size() > 0) {
									arenas.pop_back();
								}

								text_tooltip(arenas);
							}
						}
					};

					const bool should_download = do_big_button(dl_label, icon, rgba(100, 255, 100, 255), {
						rgba(10, 50, 10, 255),
						rgba(20, 70, 20, 255),
						rgba(20, 90, 20, 255)
					}, after_cb);

					if (should_download || launch_download_on_last_selected) {
						launch_download_on_last_selected = false;
						headless.launch_download(downloadable_arenas, parsed_url(in.external_arena_files_provider));
					}
				}
			}
		}
	}

	headless.finalize_download();

	if (headless.download_failed_popup.has_value()) {
		if (headless.download_failed_popup->perform()) {
			headless.download_failed_popup = std::nullopt;
		}
	}

	return address_modified;
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
	return future_response.valid();
}

bool map_catalogue_gui_state::refresh_in_progress() const {
	return headless.list_refresh_in_progress() || future_downloaded_miniatures.valid();
}

void map_catalogue_gui_state::refresh(const address_string_type address) {
	if (refresh_in_progress()) {
		return;
	}

	selected_arenas.clear();
	last_selected = nullptr;

	headless.refresh(address);
}

void headless_map_catalogue::refresh(const address_string_type address) {
	using namespace httplib;
	using namespace httplib_utils;

	if (downloading.has_value() || list_refresh_in_progress()) {
		return;
	}

	map_list.clear();
	list_catalogue_error = {};

	future_response = launch_async(
		[&last_error = this->list_catalogue_error, officials = this->official_names, address]() -> std::vector<map_catalogue_entry> {
			if (const auto parsed = parsed_url(address); parsed.valid()) {
				const auto ca_path = CA_CERT_PATH;
				auto client = http_client_type(parsed.host);

#if BUILD_OPENSSL
				client.set_ca_cert_path(ca_path.c_str());
				client.enable_server_certificate_verification(true);
#endif
				client.set_follow_location(true);
				client.set_read_timeout(3);
				client.set_write_timeout(3);
				client.set_keep_alive(true);

				const auto location = parsed.location + "?format=json";

				auto response = client.Get(location.c_str());

				if (response == nullptr) {
					last_error = "Response was null.";
					return {};
				}

				if (!successful(response->status)) {
					last_error = typesafe_sprintf("Request failed with status: %x", response->status);
					return {};
				}

				try {
					auto result = augs::from_json_string<std::vector<map_catalogue_entry>>(response->body);

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

			last_error = typesafe_sprintf("Couldn't parse URL: %x", address);
			return {};
		}
	);
}
