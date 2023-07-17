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

constexpr auto miniature_size_v = 80;

struct map_catalogue_gui_internal {
	//std::optional<arena_downloading_session> session;
	//https_file_downloader downloader;
};

map_catalogue_gui_state::~map_catalogue_gui_state() = default;

map_catalogue_gui_state::map_catalogue_gui_state(const std::string& title) 
	: base(title), data(std::make_unique<map_catalogue_gui_internal>()) 
{

}

std::mutex miniature_mutex;

void map_catalogue_gui_state::request_miniatures(const map_catalogue_input in) {
	using namespace httplib_utils;

	completed_miniatures.clear();
	completed_miniatures.resize(map_list.size());

	has_next_miniature.store(0);

	future_downloaded_miniatures = launch_async([
		&miniature_counter = this->has_next_miniature,
		&result = this->completed_miniatures,
		for_maps = this->map_list,
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

std::string sanitize_arena_short_description(std::string in);

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
	do_column("Last updated");

	ImGui::Separator();

	const auto line_h = ImGui::GetTextLineHeight();

	const auto downloads_directory = augs::path_type(DOWNLOADED_ARENAS_DIR);

	auto process_entry = [&](const auto& entry) {
		const auto arena_name = entry.name;
		const auto arena_folder_path = downloads_directory / arena_name;
		const auto paths = editor_project_paths(arena_folder_path);

		const auto sanitized = sanitization::sanitize_arena_path(downloads_directory, paths.arena_name);
		const auto sanitized_path = std::get_if<augs::path_type>(&sanitized);

		if (!sanitized_path || *sanitized_path != arena_folder_path) {
			return;
		}

		if (filter.IsActive() && !filter.PassFilter(arena_name.c_str()) && !filter.PassFilter(entry.author.c_str())) {
			return;
		}

		const auto selectable_size = ImVec2(0, 1 + miniature_size_v);
		auto id = scoped_id(arena_name.c_str());

		const bool is_selected = found_in(selected_arenas, arena_name);

		const auto local_pos = ImGui::GetCursorPos();

		{
			auto darkened_selectables = scoped_selectable_colors({
				rgba(255, 255, 255, 20),
				rgba(255, 255, 255, 30),
				rgba(255, 255, 255, 60)
			});

			if (ImGui::Selectable("##Entry", is_selected, ImGuiSelectableFlags_SpanAllColumns, selectable_size)) {
				if (ImGui::GetIO().KeyCtrl) {
					if (found_in(selected_arenas, arena_name)) {
						selected_arenas.erase(arena_name);
					}
					else {
						selected_arenas.emplace(arena_name);
					}
				}
				else {
					selected_arenas.clear();
					selected_arenas.emplace(arena_name);
				}
			}

			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::Selectable("Reveal in explorer")) {
					in.window.reveal_in_explorer(*sanitized_path);
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
		text_disabled(" - by " + entry.author);

		ImGui::SetCursorPosY(prev_y);
		ImGui::SetCursorPosX(x);

		text_disabled(entry.short_description);

		ImGui::SetCursorPos(after_pos);

		if (num_columns > 1) {
			ImGui::NextColumn();

			const auto date = augs::date_time::from_utc_timestamp(entry.version_timestamp);

			const auto ago = typesafe_sprintf(
				"%x\n%x", 
				date.get_readable_day_hour(),
				date.how_long_ago_tell_seconds()
			);

			text_disabled(ago);

			ImGui::NextColumn();
		}
	};

	auto& entries = map_list;

	if (sort_by_column == 0) {
		::in_order_of(
			entries,
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
			entries,
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

	bool mofified = false;

	const auto couldnt_download = std::string("Couldn't download the server list.\n");

	if (!refreshed_once) {
		refreshed_once = true;
		refresh(in);
	}

	if (valid_and_is_ready(future_response)) {
		map_list = future_response.get();
		focus_on_filter_once = true;

		request_miniatures(in);
	}

	if (valid_and_is_ready(future_downloaded_miniatures)) {
		future_downloaded_miniatures.get();
	}

	{
		if (input_text("Provider", in.external_arena_files_provider)) {

		}

		if (ImGui::IsItemDeactivatedAfterEdit()) {
			mofified = true;
			refresh(in);
		}
	}

	{
		ImGui::SameLine();

		auto scope = maybe_disabled_cols({}, refresh_in_progress());

		if (ImGui::Button("Refresh")) {
			refresh(in);
		}
	}

	if (list_refresh_in_progress()) {
		text_color("Downloading catalogue...", yellow);
	}
	else {
		if (last_error.size() > 0) {
			text_color("Failed to download the catalogue.\n", red);
			text_color(last_error, red);
		}
		else {
			if (focus_on_filter_once) {
				focus_on_filter_once = false;
				ImGui::SetKeyboardFocusHere(0);
			}

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

			auto do_big_button = [&](auto label, auto icon, auto col, std::array<rgba, 3> bg_cols) {
				return selectable_with_icon(
					in.necessary_images[icon],
					label,
					button_size_mult,
					vec2(1.0f, button_padding_mult),
					col,
					bg_cols
				);
			};

			const auto icon = assets::necessary_image_id::DOWNLOAD_ICON;

			if (selected_arenas.empty()) {
				auto cols = maybe_disabled_cols({}, true);

				do_big_button("No map selected.", icon, rgba(100, 100, 100, 255), {
					rgba(25, 25, 25, 255),
					rgba(50, 50, 50, 255),
					rgba(50, 50, 50, 255)
				});
			}
			else {
				auto dl_label = typesafe_sprintf("Download %x maps", selected_arenas.size());

				if (selected_arenas.size() == 1) {
					dl_label = "Download " + *selected_arenas.begin();
				}

				do_big_button(dl_label, icon, rgba(100, 255, 100, 255), {
					rgba(10, 50, 10, 255),
					rgba(20, 70, 20, 255),
					rgba(20, 90, 20, 255)
				});
			}

		}
	}

	return mofified;
}

bool map_catalogue_gui_state::list_refresh_in_progress() const {
	return future_response.valid();
}

bool map_catalogue_gui_state::refresh_in_progress() const {
	return list_refresh_in_progress() || future_downloaded_miniatures.valid();
}

void map_catalogue_gui_state::refresh(const map_catalogue_input in) {
	using namespace httplib;
	using namespace httplib_utils;

	if (refresh_in_progress()) {
		return;
	}

	map_list.clear();

	last_error = {};

	future_response = launch_async(
		[this, address = in.external_arena_files_provider]() -> std::vector<map_catalogue_entry> {
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

					for (auto& r : result) {
						r.short_description = sanitize_arena_short_description(r.short_description);
					}

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

