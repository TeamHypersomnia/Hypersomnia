#pragma once
#include <vector>
#include <optional>
#include <unordered_map>
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/imgui/simple_browse_path_tree.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/misc/maybe_official_path.h"
#include "application/setups/editor/gui/property_editor/browsed_path_entry_base.h"
#include "application/setups/editor/gui/property_editor/widgets/keyboard_acquiring_popup.h"
#include "view/asset_funcs.h"
#include "augs/misc/date_time.h"
#include "augs/log.h"
#include "augs/misc/readable_bytesize.h"
#include "augs/readwrite/stream_read_error.h"

class demo_chooser : keyboard_acquiring_popup {
	using I = std::string;

	using base = keyboard_acquiring_popup;

	using asset_widget_path_entry = browsed_path_entry_base<I>;

	std::vector<asset_widget_path_entry> all_paths;
	path_tree_settings tree_settings;

	struct path_meta {
		std::string write_time = "Unknown";
		std::string server_name = "Unknown";
	};

	std::unordered_map<augs::path_type, path_meta> path_metas;

public:
	template <class F>
	void perform(
		const std::string& label, 
		const augs::path_type& current_source,
		const augs::path_type& demo_folder_path,
		F on_choice
	) {
		using namespace augs::imgui;

		const auto displayed_str = current_source.empty() ? std::string("(None selected)") : current_source.string();

		auto scoped_w = augs::imgui::scoped_item_width(ImGui::GetWindowWidth() - ImGui::CalcTextSize(label.c_str()).x - 20);

		if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			if (base::check_opened_first_time()) {
				path_metas.clear();
				all_paths.clear();

				auto path_adder = [this](const auto& full_path) {
					if (full_path.extension() != ".demc") {
						return callback_result::CONTINUE;
					}

					maybe_official_path<I> entry;
					entry.path = full_path;

					path_meta meta;

					try {
						auto t = augs::open_binary_input_stream(full_path);
						demo_file_meta file_meta;
						augs::read_bytes(t, file_meta);

						meta.server_name = file_meta.server_name;
						meta.write_time = augs::date_time::from_utc_timestamp(file_meta.when_recorded).how_long_ago();
					}
					catch (const std::ifstream::failure&) {

					}
					catch (const augs::stream_read_error&) {

					}
					catch (const augs::filesystem_error&) {

					}

					path_metas[full_path] = meta;

					all_paths.emplace_back(std::move(entry));
					return callback_result::CONTINUE;
				};

				{
					if (augs::exists(demo_folder_path)) {
						augs::for_each_in_directory(
							demo_folder_path,
							[](const auto&) { return callback_result::CONTINUE; },
							path_adder
						);
					}
				}

				sort_range(all_paths);
				reverse_range(all_paths);
			}

			const bool acquire_keyboard = base::pop_acquire_keyboard();

			auto path_callback = [&](const auto& path_entry, const auto displayed_name) {
				const auto button_path = path_entry.get_full_path();
				const bool is_current = button_path.path == current_source;

				if (is_current && acquire_keyboard) {
					ImGui::SetScrollHereY();
				}

				if (ImGui::Selectable(displayed_name.c_str(), is_current)) {
					ImGui::CloseCurrentPopup();

					LOG("Choosing button path: %x ", button_path);
					on_choice(button_path);
				}
			};

			{
				using namespace augs::imgui;

				thread_local ImGuiTextFilter filter;
				filter.Draw();

				if (acquire_keyboard) {
					ImGui::SetKeyboardFocusHere();
				}

				auto files_view = scoped_child("Files view", ImVec2(0, 20 * ImGui::GetTextLineHeightWithSpacing()));

				ImGui::Columns(4);
				ImGui::SetColumnWidth(0, ImGui::CalcTextSize("99.99.99999 at 99-99-99.dem").x);
				ImGui::SetColumnWidth(1, ImGui::CalcTextSize("999999 years ago").x);
				ImGui::SetColumnWidth(2, ImGui::CalcTextSize("9999999999999 MB").x);
				ImGui::SetColumnWidth(3, ImGui::CalcTextSize("arena-blahblah.hypersomnia.io").x);

				text_disabled("Filename");
				ImGui::NextColumn();
				text_disabled("Recorded");
				ImGui::NextColumn();
				text_disabled("File size");
				ImGui::NextColumn();
				text_disabled("Server");
				ImGui::NextColumn();

				ImGui::Separator();

				for (const auto& l : all_paths) {
					const auto filename = l.get_filename().string();
					const auto& full_path = l.get_full_path().path;
					const auto path_meta = path_metas.at(full_path);

					const bool passes_anything = 
						filter.PassFilter(filename.c_str())
						||  filter.PassFilter(path_meta.server_name.c_str())
						||  filter.PassFilter(path_meta.write_time.c_str())
					;

					if (!passes_anything) {
						continue;
					}

					path_callback(l, filename);
					ImGui::NextColumn();
					text_disabled(path_meta.write_time);
					ImGui::NextColumn();
					text_disabled(readable_bytesize(augs::get_file_size(l.get_full_path().path)));
					ImGui::NextColumn();

					{
						const auto displayed_server_name = path_meta.server_name.empty() ? std::string("Custom game server") : path_meta.server_name;
						text_disabled(displayed_server_name);
					}

					ImGui::NextColumn();
				}
			}
		}
		else {
			base::mark_not_opened();
		}
	}
};
