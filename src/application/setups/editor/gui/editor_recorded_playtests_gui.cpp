#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/templates/chrono_templates.h"
#include "augs/window_framework/window.h"
#include "application/setups/editor/gui/editor_recorded_playtests_gui.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording_files.h"
#include "application/setups/editor/editor_setup.h"

void editor_recorded_playtests_gui::perform(const editor_recorded_playtests_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	if (rescan_requested) {
		rescan_requested = false;
		entries = ::find_recorded_playtests(in.setup.get_recorded_playtests_folder());
	}

	const auto max_icon_size = ImGui::GetTextLineHeight();

	if (ImGui::Button("Refresh")) {
		rescan_requested = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Open folder")) {
		const auto folder = in.setup.get_recorded_playtests_folder();
		augs::create_directories(folder);
		in.window.reveal_in_explorer(folder);
	}

	if (entries.empty()) {
		text_disabled("Every playtest (Space) is recorded here -\nreplay it to watch or render it again.");
		return;
	}

	thread_local ImGuiTextFilter filter;
	filter_with_hint(filter, "##RecordedPlaytestsFilter", "Search recordings...");

	auto child = scoped_child("recordings view");

	const auto avail = ImGui::GetContentRegionAvail().x;
	const auto spacing = ImGui::GetStyle().ItemSpacing;

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, avail - max_icon_size - spacing.x * 2);

	std::optional<augs::path_type> replay_request;
	std::optional<editor_recorded_playtest_entry> delete_request;

	auto finish_renaming = [&](editor_recorded_playtest_entry& entry) {
		entry.meta.name = rename_buffer;

		try {
			augs::save_as_json(entry.meta, entry.paths.meta_json);
		}
		catch (const std::exception& err) {
			LOG("Could not rename the recorded playtest %x: %x", entry.paths.folder, err.what());
		}

		renamed_folder = std::nullopt;
	};

	for (auto& entry : entries) {
		const auto label = entry.get_display_name();
		const auto details = typesafe_sprintf(
			"%x  %x  %x",
			::format_mins_secs(entry.meta.duration_secs),
			entry.meta.game_mode.empty() ? std::string("Quick test") : entry.meta.game_mode,
			entry.get_recorded_at().how_long_ago_brief()
		);

		if (filter.IsActive() && !filter.PassFilter(label.c_str()) && !filter.PassFilter(details.c_str())) {
			continue;
		}

		auto id_scope = scoped_id(entry.paths.folder.string().c_str());

		const bool is_selected = selected_folder == entry.paths.folder;
		const bool is_renamed = renamed_folder.has_value() && *renamed_folder == entry.paths.folder;

		if (is_renamed) {
			if (rename_focus_once) {
				ImGui::SetKeyboardFocusHere();
				rename_focus_once = false;
			}

			if (input_text<100>("##RecordingName", rename_buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
				finish_renaming(entry);
			}
			else if (!ImGui::IsItemActive() && !ImGui::IsItemFocused()) {
				renamed_folder = std::nullopt;
			}
		}
		else {
			const auto selectable_label = typesafe_sprintf("%x###Recording", label);

			if (ImGui::Selectable(selectable_label.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
				selected_folder = entry.paths.folder;

				if (ImGui::IsMouseDoubleClicked(0)) {
					replay_request = entry.paths.folder;
				}
			}

			if (ImGui::IsItemHovered()) {
				const auto by_whom = entry.meta.nickname.empty() ? std::string() : " by " + entry.meta.nickname;

				text_tooltip(
					"Recorded %x%x, game version %x.\nDouble-click to replay, right-click for more.",
					entry.get_recorded_at().get_readable(),
					by_whom,
					entry.meta.game_version
				);
			}

			if (ImGui::BeginPopupContextItem()) {
				selected_folder = entry.paths.folder;

				if (ImGui::Selectable("Replay")) {
					replay_request = entry.paths.folder;
				}

				if (ImGui::Selectable("Rename")) {
					renamed_folder = entry.paths.folder;
					rename_buffer = entry.meta.name;
					rename_focus_once = true;
				}

				if (ImGui::Selectable("Reveal in explorer")) {
					in.window.reveal_in_explorer(entry.paths.meta_json);
				}

				ImGui::Separator();

				{
					auto colored = scoped_text_color(rgba(220, 80, 80, 255));

					if (ImGui::Selectable("Delete")) {
						delete_request = entry;
					}
				}

				ImGui::EndPopup();
			}

			text_disabled(details);
		}

		ImGui::NextColumn();

		{
			const auto play_icon = in.necessary_images[assets::necessary_image_id::EDITOR_TOOL_PLAYTEST];
			const auto scaled_icon_size = vec2::scaled_to_max_size(play_icon.get_original_size(), max_icon_size);
			const auto dim = rgba(255, 255, 255, 70);
			const auto cols = is_selected ? colors_nha::standard() : colors_nha { dim, rgba(255, 255, 255, 200), white };

			if (game_image_button("###ReplayRecording", play_icon, scaled_icon_size, cols, augs::imgui_atlas_type::GAME)) {
				selected_folder = entry.paths.folder;
				replay_request = entry.paths.folder;
			}

			if (ImGui::IsItemHovered()) {
				text_tooltip("Replay");
			}
		}

		ImGui::NextColumn();
	}

	ImGui::Columns(1);

	if (delete_request.has_value()) {
		try {
			::remove_recorded_playtest_folder(delete_request->paths);
		}
		catch (const std::exception& err) {
			LOG("Could not delete the recorded playtest %x: %x", delete_request->paths.folder, err.what());
		}

		rescan_requested = true;
	}

	if (replay_request.has_value()) {
		in.setup.get_playtest_recordings().start_replaying(in.setup, *replay_request);
	}
}
