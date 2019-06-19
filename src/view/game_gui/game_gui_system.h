#pragma once
#include <unordered_map>

#include "augs/misc/timing/delta.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/cosmic_entropy.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/spells/all_spells.h"

#include "view/game_gui/game_gui_context.h"
#include "view/game_gui/elements/game_gui_root.h"
#include "view/game_gui/inventory_gui_intent_type.h"

#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/slot_button.h"
#include "view/game_gui/elements/item_button.h"
#include "game/detail/inventory/wielding_setup.h"

#include "game/detail/view_input/predictability_info.h"

struct game_gui_post_solve_settings {
	prediction_input prediction;
};

class game_gui_system {
public:
	std::unordered_map<entity_id, item_button> item_buttons;
	std::unordered_map<inventory_slot_id, slot_button> slot_buttons;

	std::unordered_map<entity_id, character_gui> character_guis;

	using pending_entropy_type = cosmic_player_entropy;

	entity_id recently_dropped;
	pending_entropy_type pending;
	
	game_gui_rect_world world;
	game_gui_rect_tree tree;
	game_gui_root root;

	game_gui_context create_context(
		const vec2i screen_size,
		const augs::event::state input_state,
		const const_entity_handle gui_entity,
		const game_gui_context_dependencies deps
	) {
		return {
			{ world, tree, screen_size, input_state },
			*this,
			gui_entity,
			get_character_gui(gui_entity),
			deps
		};
	}

	const_game_gui_context create_context(
		const vec2i screen_size,
		const augs::event::state input_state,
		const const_entity_handle gui_entity,
		const game_gui_context_dependencies deps
	) const {
		return {
			{ world, tree, screen_size, input_state },
			*this,
			gui_entity,
			get_character_gui(gui_entity),
			deps
		};
	}

	pending_entropy_type get_and_clear_pending_events();

	void queue_transfer(const entity_id&, const item_slot_transfer_request);
	void queue_wielding(const entity_id&, const wielding_setup&);

	character_gui& get_character_gui(const entity_id);
	const character_gui& get_character_gui(const entity_id) const;

	slot_button& get_slot_button(const inventory_slot_id);
	const slot_button& get_slot_button(const inventory_slot_id) const;

	item_button& get_item_button(const entity_id);
	const item_button& get_item_button(const entity_id) const;

	bool control_gui_world(
		const game_gui_context context,
		const augs::event::change change
	);

	void control_hotbar_and_action_button(
		const const_entity_handle root_entity,
		const inventory_gui_intent intent
	);

	void advance(
		const game_gui_context context,
		const augs::delta dt
	);

	void build_tree_data(const game_gui_context);

	void rebuild_layouts(
		const game_gui_context context
	);

	void reserve_caches_for_entities(const size_t) const {}
	void standard_post_solve(
		const_logic_step, 
		game_gui_post_solve_settings
	);
};