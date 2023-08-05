#pragma once
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "game/enums/faction_type.h"
#include "augs/templates/type_in_list_id.h"
#include "game/modes/arena_submodes.h"
#include "application/setups/editor/project/editor_layer_id.h"

struct editor_firearm_resource;
struct editor_melee_resource;
struct editor_explosive_resource;

struct editor_mode_common {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_mode_common
	std::vector<editor_layer_id> activate_layers;
	std::vector<editor_layer_id> deactivate_layers;
	// END GEN INTROSPECTOR

	bool operator==(const editor_mode_common&) const = default;
};

struct editor_requested_equipment {
	// GEN INTROSPECTOR struct editor_requested_equipment
	editor_typed_resource_id<editor_firearm_resource> firearm;
	editor_typed_resource_id<editor_melee_resource> melee;
	editor_typed_resource_id<editor_explosive_resource> explosive;

	bool all_spells = false;
	bool backpack = false;
	bool electric_armor = false;
	uint32_t extra_ammo_pieces = 2;

	uint32_t num_explosives = 1;

	float haste_time = 0.0f;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return backpack || firearm.is_set() || melee.is_set() || explosive.is_set();
	}

	bool operator==(const editor_requested_equipment&) const = default;
};

struct editor_bomb_defusal_mode {
	using subrules_type = bomb_defusal_rules;
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_bomb_defusal_mode
	uint32_t max_team_score = 16;

	uint32_t warmup_time = 81;
	uint32_t freeze_time = 10;
	uint32_t buy_time = 30;
	uint32_t round_time = 120;
	uint32_t round_end_time = 5;

	uint32_t respawn_after_ms = 0;
	uint32_t spawn_protection_ms = 0;

	per_actual_faction<editor_requested_equipment> warmup_equipment;
	per_actual_faction<editor_requested_equipment> round_start_equipment;
	// END GEN INTROSPECTOR

	bool operator==(const editor_bomb_defusal_mode&) const = default;

	static auto get_identifier() {
		return "bomb_defusal";
	}

	static auto get_display_name() {
		return subrules_type::get_name();
	}
};

struct editor_gun_game_mode {
	using subrules_type = gun_game_rules;
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_gun_game_mode
	uint32_t max_team_score = 2;

	uint32_t warmup_time = 81;
	uint32_t freeze_time = 5;
	uint32_t round_time = 60 * 20;
	uint32_t round_end_time = 5;

	uint32_t respawn_after_ms = 1000;
	uint32_t spawn_protection_ms = 3000;

	per_actual_faction<editor_requested_equipment> warmup_equipment;

	per_actual_faction<editor_requested_equipment> basic_equipment;
	per_actual_faction<editor_requested_equipment> final_equipment;

	bool can_throw_melee_on_final_level = false;

	std::vector<editor_typed_resource_id<editor_firearm_resource>> progression;
	// END GEN INTROSPECTOR

	bool operator==(const editor_gun_game_mode&) const = default;

	static auto get_identifier() {
		return "gun_game";
	}

	static auto get_display_name() {
		return subrules_type::get_name();
	}
};

struct editor_quick_test_mode {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_quick_test_mode
	per_actual_faction<editor_requested_equipment> equipment;
	uint32_t respawn_time_ms = 1000;
	// END GEN INTROSPECTOR

	bool operator==(const editor_quick_test_mode&) const = default;

	static auto get_identifier() {
		return "quick_test";
	}

	static auto get_display_name() {
		return "Test mode";
	}
};

struct editor_game_mode_resource_editable {
	// GEN INTROSPECTOR struct editor_game_mode_resource_editable
	editor_mode_common common;

	editor_quick_test_mode quick_test;
	editor_bomb_defusal_mode bomb_defusal;
	editor_gun_game_mode gun_game;
	// END GEN INTROSPECTOR

	bool operator==(const editor_game_mode_resource_editable&) const = default;
};

using editor_all_game_modes = type_list<
	editor_quick_test_mode,
	editor_bomb_defusal_mode,
	editor_gun_game_mode
>;

using editor_game_mode_id = type_in_list_id<editor_all_game_modes>;

struct editor_game_mode_resource {
	using editable_type = editor_game_mode_resource_editable;

	editor_game_mode_id type;
	editable_type editable;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Game mode";
	}
};
