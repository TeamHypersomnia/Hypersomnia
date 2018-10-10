#include "application/setups/editor/gui/editor_player_gui.h"
#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_player.hpp"
#include "application/setups/editor/editor_folder.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/templates/chrono_templates.h"
#include "augs/misc/readable_bytesize.h"

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
		const auto total = player.get_total_secs();
		const auto current = player.get_current_secs();

		if (total > 0.0) {
			//text("Player position:");

			auto mult = static_cast<float>(current / total);
			mult = std::clamp(mult, 0.f, 1.f);

			if (slider("Player position", mult, 0.f, 1.f)) {
				mult = std::clamp(mult, 0.f, 1.f);

				const auto target_step = player.get_total_steps() * mult;
				PLR_LOG_NVPS(mult, player.get_current_step(), player.get_total_steps(), target_step);

				if (player.is_recording()) {
					player.begin_replaying(folder);
				}

				player.seek_to(target_step, cmd_in);
			}

			ImGui::ProgressBar(mult);
		}

		text("Current step: %x/%x", player.get_current_step(), player.get_total_steps());
		text("Current time: %x", format_mins_secs_ms(current));
		text("Recording length: %x", format_mins_secs_ms(total));

		const auto& snapshots = player.get_snapshots();

		std::size_t total_snapshot_bytes = 0;

		for (const auto& s : snapshots) {
			total_snapshot_bytes += s.second.size();
		}

		text("Snapshots: %x (%x)", snapshots.size(), readable_bytesize(total_snapshot_bytes));

		if (snapshots.size() > 0) {
			auto it = snapshots.upper_bound(player.get_current_step());
			--it;

			const auto dist = std::distance(snapshots.begin(), it);
			text("Current snapshot: %x (step: %x)", dist, (*it).first); 
		}
	}

	text("(Debug) player dirty: %x", player.dirty);
}
