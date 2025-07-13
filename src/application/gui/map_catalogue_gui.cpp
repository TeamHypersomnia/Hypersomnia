#include <cstddef>
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
#include "augs/misc/date_time.h"
#include "augs/templates/in_order_of.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "application/setups/editor/project/editor_project_meta.h"
#include "augs/readwrite/to_bytes.h"
#include "application/gui/headless_map_catalogue.hpp"

auto get_miniatures_directory() { 
	return CACHE_DIR / "miniatures";
}

constexpr auto miniature_size_v = 80;
#if WEB_LOWEND
constexpr auto preview_size_v = 150;
#else
constexpr auto preview_size_v = 400;
#endif

editor_project_meta read_meta_from(const augs::path_type& arena_folder_path);

map_catalogue_gui_state::map_catalogue_gui_state(const std::string& title) : base(title) {}
map_catalogue_gui_state::~map_catalogue_gui_state() = default;

std::mutex miniature_mutex;

void map_catalogue_gui_state::request_miniatures(const map_catalogue_input in) {
	using namespace httplib_utils;

	last_miniatures.clear();
	miniature_downloader.reset();

	if (const auto parsed = parsed_url(in.external_arena_files_provider); parsed.valid()) {
		augs::create_directories(get_miniatures_directory());

		miniature_downloader.emplace(parsed);

		for (const auto& m : headless.get_map_list()) {
			m.miniature_id = 0;

			const auto location = typesafe_sprintf("%x/miniature.png", m.name);
			miniature_downloader->download_file(location);
		}

		rebuild_miniatures();
	}
}

void map_catalogue_gui_state::rebuild_miniatures() {
	mark_rebuild_miniatures = true;
}

std::optional<std::vector<ad_hoc_atlas_subject>> map_catalogue_gui_state::get_new_ad_hoc_images() {
	if (mark_rebuild_miniatures) {
		mark_rebuild_miniatures = false;
		return last_miniatures;
	}

	return std::nullopt;
}

std::string sanitize_arena_short_description(std::string in);

using S = map_catalogue_entry::state;

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
		const auto max_w = ImGui::CalcTextSize("999 days ago 99").x;
		const auto ago_col_w = std::max(max_w, avail.x * 0.2f);
		const auto first_col_w = avail.x - ago_col_w;

		ImGui::SetColumnWidth(0, first_col_w);
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

		if (only_playable && !entry.playable) {
			entry.passed_filter = false;
			return;
		}

		if (filter.IsActive() && !filter.PassFilter(arena_name.c_str()) && !filter.PassFilter(entry.author.c_str())) {
			entry.passed_filter = false;
			return;
		}
		else {
			entry.passed_filter = true;
		}

		const auto selectable_size = ImVec2(0, 1 + miniature_size_v);
		auto id = scoped_id(arena_name.c_str());

		auto entry_id = entry.name;
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
						last_selected = "";
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

#if !PLATFORM_WEB
					ImGui::Separator();

					auto scope = maybe_disabled_cols({}, entry.get_state() == S::NOT_FOUND);

					if (ImGui::Selectable("Reveal in explorer")) {
						in.window.reveal_in_explorer(arena_folder_path);
					}
#endif
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

#if !WEB_LOWEND
		if (!in.streamer_mode) {
			text_disabled("- by " + entry.author);
		}
#endif

		ImGui::SetCursorPosY(prev_y);
		ImGui::SetCursorPosX(x);

		if (!in.streamer_mode) {
			ImGui::PushTextWrapPos();
			text_disabled(entry.short_description);
			ImGui::PopTextWrapPos();
		}

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

void map_catalogue_gui_state::request_rescan() {
	headless.request_rescan();
}

void map_catalogue_gui_state::request_refresh() {
	headless.request_refresh();
}

const map_catalogue_entry* map_catalogue_gui_state::find_entry_by(const std::string& name) {
	for (auto& m : headless.get_map_list()) {
		if (m.name == name) {
			return std::addressof(m);
		}
	}

	return nullptr;
}

bool map_catalogue_gui_state::perform(const map_catalogue_input in) {
	using namespace httplib_utils;
	using namespace augs::imgui;

	if (!show) {
		return false;
	}

	center_flag = ImGuiCond_Always;

	if (ImGui::GetIO().DisplaySize.x <= 1200) {
		centered_size_mult = vec2(0.97f, 0.97f);
	}
	else {
		centered_size_mult = vec2(0.9f, 0.8f);
	}

	auto imgui_window = make_scoped_window(ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

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

	if (miniature_downloader && !miniature_downloader->is_running()) {
		miniature_downloader.reset();
		rebuild_miniatures();
	}

	if (miniature_downloader) {
		if (auto next_miniature = miniature_downloader->get_downloaded_file()) {
			const auto& map_list = headless.get_map_list();

			const auto current_miniature_index = last_miniatures.size();

			if (current_miniature_index < map_list.size()) {
				LOG("Finished downloading miniature %x.", current_miniature_index);

				const auto this_id = static_cast<ad_hoc_entry_id>(1 + current_miniature_index);

				const auto miniature_filename = typesafe_sprintf("%x.png", this_id);
				const auto next_path = get_miniatures_directory() / miniature_filename;

				const auto new_subject = ad_hoc_atlas_subject { this_id, next_path };
				last_miniatures.push_back(new_subject);

				augs::save_string_as_bytes(next_miniature->second, next_path);

				map_list[current_miniature_index].miniature_id = this_id;

#if WEB_LOWEND
				if (current_miniature_index == 7) {
					/* Only rebuild the first few visible and then wait until completion */
					rebuild_miniatures();
				}
#else
				rebuild_miniatures();
#endif
			}

			if (last_miniatures.size() == map_list.size()) {
				LOG("Finished downloading all miniatures.");
				miniature_downloader.reset();
				rebuild_miniatures();
			}
		}
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

		ImGui::SameLine();
		ImGui::Checkbox("Only playable", &only_playable);
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
						const auto entry_id = entry.name;
						const bool is_selected = found_in(selected_arenas, entry_id);

						if (!is_selected) {
							all_selected_already = false;

							break;
						}
					}
				}

				for (auto& entry : headless.get_map_list()) {
					const auto entry_id = entry.name;

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

				auto selected_entry = find_entry_by(last_selected);

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

						if (!in.streamer_mode) {
							text_disabled(std::string("By: ") + entry.author);

							text_color(entry.short_description, rgba(210, 210, 210, 255));
						}

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
			   
				for (const auto& map : headless.get_map_list()) {
					if (found_in(selected_arenas, map.name)) {
						if (map.get_state() != S::ON_DISK) {
							downloadable_arenas.push_back({ map.name, map.version_timestamp, map.last_displayed_index });
						}
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

bool map_catalogue_gui_state::refresh_in_progress() const {
	return headless.list_refresh_in_progress() || miniature_downloader.has_value();
}

void map_catalogue_gui_state::refresh(const address_string_type address) {
	if (refresh_in_progress()) {
		return;
	}

	selected_arenas.clear();
	last_selected = "";

	headless.refresh(address);
}

