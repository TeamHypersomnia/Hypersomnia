#pragma once
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "game/enums/faction_type.h"
#include "augs/templates/type_in_list_id.h"

struct editor_firearm_resource;
struct editor_melee_resource;
struct editor_explosive_resource;

struct editor_requested_equipment {
	// GEN INTROSPECTOR struct editor_requested_equipment
	editor_typed_resource_id<editor_firearm_resource> firearm;
	editor_typed_resource_id<editor_melee_resource> melee;
	editor_typed_resource_id<editor_explosive_resource> explosive;

	bool backpack = false;
	bool electric_armor = false;
	uint32_t extra_ammo_pieces = 2;

	uint32_t num_explosives = 1;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return backpack || firearm.is_set() || melee.is_set() || explosive.is_set();
	}

	bool operator==(const editor_requested_equipment&) const = default;
};

struct editor_bomb_defusal_mode {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_bomb_defusal_mode
	uint32_t warmup_time = 81;
	uint32_t freeze_time = 10;
	uint32_t buy_time = 30;
	uint32_t round_time = 120;
	uint32_t round_end_time = 5;
	uint32_t max_team_score = 16;

	per_actual_faction<editor_requested_equipment> warmup_equipment;
	per_actual_faction<editor_requested_equipment> round_start_equipment;
	// END GEN INTROSPECTOR
};

struct editor_quick_test_mode {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_quick_test_mode
	per_actual_faction<editor_requested_equipment> equipment;
	uint32_t respawn_time_ms = 1000;
	// END GEN INTROSPECTOR
};

struct editor_game_mode_resource_editable {
	// GEN INTROSPECTOR struct editor_game_mode_resource_editable
	editor_quick_test_mode quick_test;
	editor_bomb_defusal_mode bomb_defusal;
	// END GEN INTROSPECTOR
};

struct editor_game_mode_resource {
	using editable_type = editor_game_mode_resource_editable;
	using type_type = type_in_list_id<type_list<
		editor_quick_test_mode,
		editor_bomb_defusal_mode
	>>;

	type_type type;
	editable_type editable;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Game mode";
	}
};
