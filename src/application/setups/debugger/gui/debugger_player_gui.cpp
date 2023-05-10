#include "application/setups/debugger/gui/debugger_player_gui.h"
#include "application/setups/debugger/debugger_player.h"
#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/debugger_folder.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/templates/chrono_templates.h"
#include "augs/misc/readable_bytesize.h"
#include "application/setups/debugger/debugger_settings.h"
#include "application/setups/debugger/debugger_paths.h"

#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "augs/misc/readable_bytesize.h"

#include "application/setups/debugger/detail/update_official_content.h"
#include "application/setups/debugger/commands/detail/debugger_property_accessors.h"

void debugger_player_gui::perform(const debugger_command_input cmd_in) {
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
		auto scope = maybe_disabled_cols({}, player.get_current_step() == 0);

		if (ImGui::Button("Rewind")) {
			if (player.is_recording()) {
				player.begin_replaying(folder);
			}

			player.seek_to(0, cmd_in);
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
		const auto total = player.get_total_secs(folder);
		const auto current = player.get_current_secs();

		if (total > 0.0) {
			//text("Player position:");

			auto mult = static_cast<float>(current / total);
			mult = std::clamp(mult, 0.f, 1.f);

			if (slider("Player position", mult, 0.f, 1.f)) {
				mult = std::clamp(mult, 0.f, 1.f);

				const auto target_step = player.get_total_steps(folder) * mult;

				if (player.is_recording()) {
					player.begin_replaying(folder);
				}

				player.seek_to(target_step, cmd_in);
			}

			ImGui::ProgressBar(mult);
		}

		text("Current step: %x/%x", player.get_current_step(), player.get_total_steps(folder));
		text("Current time: %x", format_mins_secs_ms(current));
		text("Recording length: %x", format_mins_secs_ms(total));
		text("Step to entropy size: %x", readable_bytesize(player.estimate_step_to_entropy_size()));

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

	{
		auto& opts = player.cosmic_recording_options;

		if (checkbox("Overwrite intents", opts.overwrite_intents)) {
			//player.set_dirty();
		}

		if (checkbox("Overwrite motions", opts.overwrite_motions)) {
			//player.set_dirty();
		}

		if (checkbox("Overwrite mode commands", player.mode_recording_options.overwrite)) {
			//player.set_dirty();
		}

		if (checkbox("Overwrite rest", opts.overwrite_rest)) {
			//player.set_dirty();
		}
	}

	{
		if (cmd_in.settings.save_entropies_to_live_file) {
			const auto paths = cmd_in.folder.get_paths();
			const auto live_file_path = paths.entropies_live_file;

			if (augs::exists(live_file_path)) {
				text_color("(Debug) entropies.live file exists.", green);

				if (ImGui::Button("Read & replay")) {
					if (player.has_testing_started()) {
						player.finish_testing(cmd_in, finish_testing_type::DISCARD_CHANGES);
					}

					if (!player.has_testing_started()) {
						player.begin_replaying(folder);
					}

					player.read_live_entropies(live_file_path);
				}

				if (ImGui::Button("Delete")) {
					augs::remove_file(live_file_path);
				}
			}
		}
		else {
			text_disabled("Writing entropies to a live file is disabled.");
		}
	}

	thread_local update_official_content_settings settings;

	checkbox("Overwrite recoils", settings.overwrite_recoils);
	checkbox("Overwrite common assets", settings.overwrite_common_assets);
	checkbox("Overwrite physical_materials", settings.overwrite_physical_materials);
	checkbox("Overwrite non-decoration flavours", settings.overwrite_non_decoration_flavours);
	checkbox("Overwrite economy vars of the first created bomb ruleset", settings.overwrite_economy_vars);
	checkbox("Overwrite whole ruleset (except bot quota and other very specific)", settings.overwrite_whole_ruleset);
	checkbox("Overwrite spells", settings.overwrite_spells);
	checkbox("Overwrite meters", settings.overwrite_meters);
	checkbox("Overwrite perks", settings.overwrite_perks);

	if (ImGui::Button("Update official content")) {
		::update_official_content(cmd_in, settings);
	}

	if (ImGui::Button("Redirect to /gfx")) {
		for (auto& img : folder.commanded->work.viewables.image_definitions) {
			if (img.source_image.is_official) {
				img.source_image.path = augs::path_type("gfx") / img.source_image.path;
			}
		}

		for (auto& snd : folder.commanded->work.viewables.sounds) {
			if (snd.source_sound.is_official) {
				snd.source_sound.path = augs::path_type("sfx") / snd.source_sound.path;
			}
		}
	}

	text_disabled(typesafe_sprintf("Current cosmos clock: %x", folder.commanded->work.world.get_clock().now.step));

	if (ImGui::Button("Force reset cosmos clock")) {
		auto& csm = folder.commanded->work.world;
		debugger_property_accessors::force_set_clock(csm, 0);
	}
}
