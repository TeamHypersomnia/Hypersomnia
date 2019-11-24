#pragma once
#include "application/gui/client/demo_player_gui.h"
#include "application/setups/editor/detail/maybe_different_colors.h"
#include "augs/templates/chrono_templates.h"
#include "augs/window_framework/window.h"
#include "augs/misc/imgui/imgui_enum_radio.h"

inline void demo_player_gui::perform(
	augs::window& window,
	client_demo_player& player
) {
	using namespace augs::imgui;

	auto scope = make_scoped_window();

	if (!scope) {
		return;
	}

	checkbox("Show spectator overlay", show_spectator_overlay);
	text("POV:"); ImGui::SameLine(); 

	if (enum_radio(shown_arena_type, true)) {
		pending_interpolation_snap = true;
	}

	ImGui::SameLine();
	text_disabled("(?)");

	if (ImGui::IsItemHovered()) {
		auto scope = scoped_tooltip();

		text_color("Referential", yellow);
		ImGui::SameLine();
		text("shows you the events as they have truly happened on the server.");

		text_color("Predicted", yellow);
		ImGui::SameLine();
		text("shows you the events as they were seen by the player who has recorded the demo,\nwith effects of network lag reproduced exactly how they occured.");
	}

	{
		auto scope = maybe_disabled_cols({}, player.get_current_step() == 0);

		if (ImGui::Button("Rewind")) {
			player.seek_to(0);
		}

		ImGui::SameLine();
	}

	{
		auto scope = maybe_disabled_cols({}, player.is_paused());

		if (ImGui::Button("Pause")) {
			player.pause();
		}

		ImGui::SameLine();
	}

	{
		auto scope = maybe_disabled_cols({}, !player.is_paused());

		if (ImGui::Button("Resume")) {
			player.resume();
		}
	}

	text_disabled("Replaying:");
	ImGui::SameLine();

	{
		const auto demo_path = player.source_path.string();
		text_color(demo_path, cyan);

		ImGui::SameLine();

		if (ImGui::Button("Reveal in explorer")) {
			window.reveal_in_explorer(demo_path);
		}
	}

	const auto current = player.get_current_secs();

	if (player.get_total_steps() > 0) {
		auto mult = static_cast<float>(player.get_current_step()) / player.get_total_steps();
		mult = std::clamp(mult, 0.f, 1.f);

		if (slider("Player position", mult, 0.f, 1.f)) {
			mult = std::clamp(mult, 0.f, 1.f);

			const auto target_step = player.get_total_steps() * mult;
			player.seek_to(target_step);
		}

		ImGui::ProgressBar(mult);
	}

	{
		auto step_str = std::to_string(player.get_current_step());

		text("Current step:");
		ImGui::SameLine();

		auto scope = scoped_item_width(ImGui::CalcTextSize("9").x * 15);

		if (input_text<256>("##Current step", step_str, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
			const auto changed_step = std::atoi(step_str.c_str());

			player.seek_to(changed_step);
		}
	}

	ImGui::SameLine();
	text("/%x", player.get_total_steps());
	text("Current time: %x", ::format_mins_secs_ms(current));

	text_disabled("Press Alt+P to toggle this window visibility.");
}
