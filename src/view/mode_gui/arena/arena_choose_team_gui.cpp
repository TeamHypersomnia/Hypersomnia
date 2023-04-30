#include "view/mode_gui/arena/arena_choose_team_gui.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "view/viewables/images_in_atlas_map.h"
#include "application/config_lua_table.h"

#include "application/app_intent_type.h"

bool arena_choose_team_gui::control(general_gui_intent_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (const auto it = mapped_or_nullptr(in.controls, key)) {
			if (*it == general_gui_intent_type::CHOOSE_TEAM) {
				show = !show;
				return true;
			}
		}

		if (show) {
			using namespace augs::event;

			auto make_choice = [&](const faction_type f) {
				key_requested_choice = f;
			};

			if (key == keys::key::A) {
				make_choice(faction_type::DEFAULT);
				return true;
			}

			if (key == keys::key::S) {
				make_choice(faction_type::SPECTATOR);
				return true;
			}

			if (key == keys::key::M) {
				make_choice(faction_type::METROPOLIS);
				return true;
			}

			if (key == keys::key::R) {
				make_choice(faction_type::RESISTANCE);
				return true;
			}
		}
	}

	return false;
}

using input_type = arena_choose_team_gui::input;

std::optional<mode_commands::team_choice> arena_choose_team_gui::perform_imgui(const input_type in) {
	using namespace augs::imgui;

	auto unset_key_choice = augs::scope_guard([this]() {
		key_requested_choice = std::nullopt;
	});

	if (!show) {
		return std::nullopt;
	}

	center_next_window(vec2::square(0.4f), ImGuiCond_Once);

	const auto window_name = "Choose your faction";
	auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar);
	centered_text(window_name);

	const auto n = in.available_factions.size();

	auto make_choice = [&](const faction_type f) {
		show = false;
		return f;
	};

	ImGui::Columns(n, nullptr, true);

	for (const auto& info : in.available_factions) {
		const auto f = info.f;

		const auto faction_label = format_enum(f);

		const bool is_full = info.max_players && info.num_players == info.max_players;
		const bool is_current = f == in.current_faction;

		{
			const auto label = faction_label + "##Button";

			const auto disabled_col = rgba(220, 110, 110, 70);

			const auto color_scheme = [&]() {
				if (is_current) {
					return augs::imgui::colors_nha { gray, gray, gray };
				}

				if (is_full) {
					return augs::imgui::colors_nha { disabled_col, disabled_col, disabled_col };
				}
				
				return augs::imgui::colors_nha::standard();
			}();

			const auto& entry = in.images_in_atlas.find_or(in.button_logos[f]).diffuse; 
			const bool button_confirmed = entry.exists() && game_image_button(label, entry, color_scheme);

			if (!is_full) {
				if (is_current) {

				}
				else {
					if (key_requested_choice.has_value()) {
						return make_choice(*key_requested_choice);
					}

					if (button_confirmed) {
						return make_choice(f);
					}
				}
			}
		}

		const auto numbers = 
			info.max_players 
			? typesafe_sprintf("(players: %x/%x)", info.num_players, info.max_players)
			: typesafe_sprintf("(players: %x)", info.num_players)
		;

		text_color(faction_label, is_full ? rgba(255, 255, 255, 70) : info.color);
		ImGui::SameLine();

		if (is_full) {
			text_color(numbers, red);
		}
		else {
			text_disabled(numbers);
		}

		if (is_current) {
			text_color("Current faction", green);
		}
		else if (is_full) {
			text_color("Faction full", red);
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
		return make_choice(faction_type::DEFAULT);
	}

	return std::nullopt;
}
