#include "application/setups/editor/gui/editor_player_gui.h"
#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_player.hpp"
#include "application/setups/editor/editor_folder.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/templates/chrono_templates.h"

#include "application/setups/editor/detail/maybe_different_colors.h"

void editor_player_gui::perform(const editor_command_input cmd_in) {
	using namespace augs::imgui;

	auto& folder = cmd_in.folder;
	auto& player = folder.player;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	acquire_keyboard_once();

	{
		auto scope = maybe_disabled_cols({}, !player.is_paused());

		if (ImGui::Button("Record")) {
			player.begin_recording(folder);
		}

		ImGui::SameLine();

		if (ImGui::Button("Replay")) {
			player.begin_replaying(folder);
		}

		ImGui::SameLine();
	}

	{
		auto scope = maybe_disabled_cols({}, player.is_paused());

		if (ImGui::Button("Pause")) {
			player.pause();
		}
	}

	if (player.has_testing_started()) {
		const auto current = player.get_current_secs();
		const auto total = player.get_total_secs();

		if (total > 0.0) {
			//text("Player position:");

			auto mult = static_cast<float>(current / total);

			if (slider("Player position", mult, 0.f, 1.f)) {
				const auto target_step = player.get_total_steps() * mult;
				player.seek_to(target_step, cmd_in);
			}

			ImGui::ProgressBar(mult);
		}

		text("Current step: %x/%x", player.get_current_step(), player.get_total_steps());
		text("Current time: %x", format_mins_secs_ms(current));
		text("Recording length: %x", format_mins_secs_ms(total));

		text("Snapshots: %x", player.get_snapshots().size());
		text("Revision when started: %x", player.get_revision_when_started_testing());
	}
}
