#include "view/mode_gui/arena/arena_choose_team_gui.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "view/viewables/images_in_atlas_map.h"
#include "application/config_lua_table.h"

#include "application/app_intent_type.h"

bool arena_choose_team_gui::control(app_ingame_intent_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (const auto it = mapped_or_nullptr(in.controls, key)) {
			if (*it == app_ingame_intent_type::CHOOSE_TEAM) {
				show = !show;
				return true;
			}
		}
	}

	return false;
}

using input_type = arena_choose_team_gui::input;

std::optional<mode_commands::team_choice> arena_choose_team_gui::perform_imgui(const input_type in) {
	using namespace augs::imgui;

	if (!show) {
		return std::nullopt;
	}

	ImGui::SetNextWindowPosCenter();

	ImGui::SetNextWindowSize((vec2(ImGui::GetIO().DisplaySize) * 0.3f).operator ImVec2(), ImGuiCond_FirstUseEver);

	const auto window_name = "Choose your faction";
	auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar);

	{
		const auto s = ImGui::CalcTextSize(window_name, nullptr, true);
		const auto w = ImGui::GetWindowWidth();
		ImGui::SetCursorPosX(w / 2 - s.x / 2);
		text(window_name);
	}

	const auto n = in.available_factions.size();

	auto make_choice = [&](const faction_type f) {
		mode_commands::team_choice choice;
		choice.target_team = f;
		show = false;
		return choice;
	};

	ImGui::Columns(n, nullptr, true);

	for (const auto& info : in.available_factions) {
		const auto f = info.f;

		if (const auto& entry = in.images_in_atlas.at(in.button_logos[f]).diffuse; entry.exists()) {
			const auto faction_label = format_enum(f);

			const bool is_full = info.num_players == info.max_players;

			{
				const auto label = faction_label + "##Button";

				const auto disabled_col = rgba(220, 110, 110, 70);

				if (game_image_button(label, entry, is_full ? augs::imgui::colors_nha { disabled_col, disabled_col, disabled_col } : augs::imgui::colors_nha::standard())) {
					return make_choice(f);
				}
			}

			const auto numbers = typesafe_sprintf("(%x/%x)", info.num_players, info.max_players);

			text_color(faction_label, is_full ? rgba(255, 255, 255, 70) : info.color);
			ImGui::SameLine();

			if (is_full) {
				text_color(numbers + " - full", red);
			}
			else {
				text_disabled(numbers);
			}
		}

		ImGui::NextColumn();
	}

	ImGui::Columns(1);
	ImGui::Separator();

	if (ImGui::Button("Spectate")) {
		return make_choice(faction_type::SPECTATOR);
	}

	ImGui::SameLine();

	if (ImGui::Button("Auto-assign")) {
		return make_choice(faction_type::COUNT);
	}

	return std::nullopt;
}
