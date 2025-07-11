#pragma once
#include <cstddef>
#include <vector>
#include <optional>
#include "game/enums/faction_type.h"
#include "game/modes/mode_commands/team_choice.h"
#include "augs/misc/enum/enum_array.h"
#include "game/assets/ids/asset_ids.h"

class images_in_atlas_map;
struct general_gui_intent_input;

struct arena_choose_team_gui {
	struct input {
		struct faction_info {
			const faction_type f;
			const rgba color;
			const std::size_t num_players;
			const std::size_t max_players;
		};

		const per_actual_faction<assets::image_id>& button_logos;
		const std::vector<faction_info>& available_factions;
		const images_in_atlas_map& images_in_atlas;
		const faction_type current_faction;
	};

	// GEN INTROSPECTOR struct arena_choose_team_gui
	bool show = true;
	// END GEN INTROSPECTOR

	std::optional<faction_type> key_requested_choice;

	/* Always initialize as hidden */

	bool control(general_gui_intent_input);
	std::optional<mode_commands::team_choice> perform_imgui(input);
};
