#pragma once
#include <cstddef>
#include <cstdint>
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

struct game_gui_input_settings;

class game_gui_system {
public:
	std::unordered_map<entity_id, item_button> item_buttons;
	std::unordered_map<inventory_slot_id, slot_button> slot_buttons;

	/*
		One hotbar state per session. The subject whose inventory drives the
		hotbar is passed in at each call site. Spectator switches "reset" the
		hotbar implicitly: rebuild_hotbar walks the new subject's inventory
		from scratch each logical step.
	*/
	character_gui local_gui;

	/*
		The hotbar is rebuilt at most once per logical step. (step, subject)
		key gates redundant work: render frames within the same step skip,
		but a spectator switch within the same step still forces a rebuild
		against the new subject - otherwise the hotbar visibly flickers on
		high-refresh displays where many render frames fall between steps.
	*/
	std::uint64_t last_rebuilt_step = static_cast<std::uint64_t>(-1);
	entity_id last_rebuilt_subject;

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
			local_gui,
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
			local_gui,
			deps
		};
	}

	pending_entropy_type get_and_clear_pending_events();

	void queue_transfer(const entity_id&, const item_slot_transfer_request);
	void queue_wielding(const entity_id&, const wielding_setup&);
	void queue_swap_hotbar_buttons(const entity_id, const entity_id);

	character_gui& get_character_gui() { return local_gui; }
	const character_gui& get_character_gui() const { return local_gui; }

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
		const inventory_gui_intent intent,
		const game_gui_input_settings&
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
		const_entity_handle subject,
		game_gui_post_solve_settings
	);

	/*
		Idempotent: gated by (step, subject) so repeated calls within the same
		logical step against the same subject short-circuit. Render-frame call
		sites use this to ensure a spectator switch reflects on the very next
		frame instead of waiting for the next logical post-solve.
	*/
	void rebuild_hotbar(const_entity_handle subject);
};