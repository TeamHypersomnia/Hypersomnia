#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/assets/ids/asset_ids.h"
#include "augs/misc/enum/enum_array.h"
#include "game/enums/battle_event.h"
#include "game/enums/faction_type.h"
#include "game/modes/mode_player_id.h"
#include "game/detail/damage_origin.h"
#include "game/detail/economy/money_type.h"
#include "game/modes/detail/item_purchase_structs.h"
#include "game/modes/detail/fog_of_war_settings.h"
#include "game/modes/arena_player_order.h"
#include "game/modes/session_id.h"
#include "view/character_hud_type.h"

using mode_entity_id = entity_id;

struct arena_migrated_player {
	// GEN INTROSPECTOR struct arena_migrated_player
	entity_name_str chosen_name;
	session_id_type id = session_id_type::dead();
	faction_type faction = faction_type::SPECTATOR;
	// END GEN INTROSPECTOR
};

struct arena_migrated_player_entry {
	mode_player_id mode_id;
	arena_migrated_player data;
};

using arena_migrated_players = std::vector<arena_migrated_player_entry>;
using player_session_data = arena_migrated_player;

struct arena_migrated_session {
	arena_migrated_players players;
	session_id_type next_session_id;
};

struct arena_mode_win {
	// GEN INTROSPECTOR struct arena_mode_win
	augs::stepped_clock when;
	faction_type winner = faction_type::SPECTATOR;
	// END GEN INTROSPECTOR

	bool was_set() const {
		return winner != faction_type::SPECTATOR;
	}
};

/*
	Storing just an id is insufficient because a player might disconnect
	during the same round.
*/

struct knockout_participant {
	// GEN INTROSPECTOR struct knockout_participant
	mode_player_id id;
	std::string name;
	faction_type faction = faction_type::SPECTATOR;
	// END GEN INTROSPECTOR
};

struct arena_mode_knockout {
	// GEN INTROSPECTOR struct arena_mode_knockout
	augs::stepped_clock when;

	knockout_participant knockouter;
	knockout_participant assist;
	knockout_participant victim;

	damage_origin origin;
	// END GEN INTROSPECTOR
};

struct arena_mode_award {
	// GEN INTROSPECTOR struct arena_mode_award
	augs::stepped_clock when;

	mode_player_id awarded_player;
	money_type amount;
	// END GEN INTROSPECTOR
};

enum class scoreboard_icon_type {
	// GEN INTROSPECTOR enum class scoreboard_icon_type
	DEATH_ICON,
	UNCONSCIOUS_ICON,
	NO_AMMO_ICON,
	BOMB_ICON,
	DEFUSE_KIT_ICON,

	COUNT
	// END GEN INTROSPECTOR
};

struct knockout_streak_def {
	// GEN INTROSPECTOR struct knockout_streak_def
	int required_knockouts = 2;

	assets::sound_id announcement_sound;
	// END GEN INTROSPECTOR
};

using knockout_streak_def_vector = std::vector<knockout_streak_def>;

struct arena_mode_view_rules {
	using theme_flavour_type = constrained_entity_flavour_id<
		invariants::continuous_sound
	>;

	// GEN INTROSPECTOR struct arena_mode_view_rules
	std::string warmup_welcome_message = "Warm up your wrists!";
	std::string start_message = "Play fair and have fun!";

	theme_flavour_type warmup_theme;

	per_actual_faction<assets::sound_id> win_themes;

	per_actual_faction<per_actual_faction<assets::sound_id>> win_sounds;
	per_actual_faction<augs::enum_array<assets::sound_id, battle_event>> event_sounds;

	knockout_streak_def_vector streak_defs;

	per_actual_faction<assets::image_id> logos;
	per_actual_faction<assets::image_id> square_logos;
	bool show_money_of_opponents = false;
	bool show_info_icons_of_opponents = false;
	character_hud_type enemy_hud_mode = character_hud_type::SMALL_HEALTH_BAR;

	augs::enum_array<assets::image_id, scoreboard_icon_type> icons;
	assets::image_id money_icon;
	per_actual_faction<assets::image_id> headshot_icons;

	fog_of_war_settings fog_of_war;
	double audiovisual_speed = 1.0;
	uint32_t can_spectate_dead_body_for_secs = 5;

	bool enable_danger_indicators = true;
	bool enable_teammate_indicators = true;
	bool enable_tactical_indicators = true;
	// END GEN INTROSPECTOR

	const knockout_streak_def* find_streak(const int current_streak_count) const {
		for (const auto& def : streak_defs) {
			if (def.required_knockouts == current_streak_count) {
				return std::addressof(def);
			}
		}

		return nullptr;
	}
};

struct arena_mode_match_result {
	std::optional<faction_type> winner;

	static auto make_tie() {
		return arena_mode_match_result();
	}

	bool is_tie() const {
		return winner == std::nullopt;
	}
};

using arena_mode_knockouts_vector = std::vector<arena_mode_knockout>;
using arena_mode_awards_vector = std::vector<arena_mode_award>;
