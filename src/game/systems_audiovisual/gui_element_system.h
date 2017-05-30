#pragma once
#include <unordered_map>
#include <map>
#include <array>

#include "game/detail/gui/game_gui_context.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmic_entropy.h"
#include "augs/graphics/renderer.h"

struct character_gui;
class viewing_step;

class gui_element_system {
public:
	std::unordered_map<entity_id, item_button> item_buttons;
	std::unordered_map<inventory_slot_id, slot_button> slot_buttons;

	std::unordered_map<entity_id, character_gui> character_guis;

	augs::container_with_small_size<std::vector<item_slot_transfer_request>, unsigned short> pending_transfers;
	augs::container_with_small_size<std::unordered_map<entity_id, spell_id>, unsigned char> spell_requests;

	bool gui_look_enabled = false;
	vec2i screen_size_for_new_characters;
	assets::game_image_id blank_image = assets::game_image_id::INVALID;

	cosmic_entropy get_and_clear_pending_events();
	void clear_all_pending_events();

	void queue_transfer(const item_slot_transfer_request);
	void queue_transfers(const wielding_result);

	character_gui& get_character_gui(const entity_id);
	const character_gui& get_character_gui(const entity_id) const;

	slot_button& get_slot_button(const inventory_slot_id);
	const slot_button& get_slot_button(const inventory_slot_id) const;

	item_button& get_item_button(const entity_id);
	const item_button& get_item_button(const entity_id) const;

	void control_gui(
		const const_entity_handle root_entity,
		std::vector<augs::window::event::change>& events
	);

	void advance_elements(
		const const_entity_handle root_entity,
		const augs::delta dt
	);

	void rebuild_layouts(
		const const_entity_handle root_entity
	);

	void handle_hotbar_and_action_button_presses(
		const const_entity_handle root_entity,
		const game_intent_vector& intents
	);

	void reserve_caches_for_entities(const size_t) const {}

	void reposition_picked_up_and_transferred_items(const const_logic_step);
	void erase_caches_for_dead_entities(const cosmos&);
};