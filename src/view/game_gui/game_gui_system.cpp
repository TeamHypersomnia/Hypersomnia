#include <cstddef>
#include "augs/templates/container_templates.h"
#include "augs/gui/button_corners.h"

#include "game/enums/slot_function.h"

#include "game/detail/inventory/inventory_slot_handle.h"

#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"

#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/item_component.h"
#include "game/messages/performed_transfer_message.h"

#include "view/game_gui/game_gui_system.h"
#include "view/game_gui/game_gui_element_location.h"
#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/slot_button.h"
#include "view/game_gui/elements/item_button.h"
#include "game/messages/changed_identities_message.h"
#include "game/detail/weapon_like.h"
#include "view/game_gui/elements/game_gui_settings.h"

#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/detail/weapon_like.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "augs/templates/logically_empty.h"
#include "game/detail/explosive/like_explosive.h"
#include "3rdparty/imgui/imgui.h"

static int to_hotbar_index(const inventory_gui_intent_type type) {
	switch (type) {
	case inventory_gui_intent_type::HOTBAR_0: return 0;
	case inventory_gui_intent_type::HOTBAR_1: return 1;
	case inventory_gui_intent_type::HOTBAR_2: return 2;
	case inventory_gui_intent_type::HOTBAR_3: return 3;
	case inventory_gui_intent_type::HOTBAR_4: return 4;
	case inventory_gui_intent_type::HOTBAR_5: return 5;
	case inventory_gui_intent_type::HOTBAR_6: return 6;
	case inventory_gui_intent_type::HOTBAR_7: return 7;
	case inventory_gui_intent_type::HOTBAR_8: return 8;
	case inventory_gui_intent_type::HOTBAR_9: return 9;
	default: return -1;
	}
}

static int to_special_action_index(const inventory_gui_intent_type type) {
	switch (type) {
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_1: return 0;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_2: return 1;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_3: return 2;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_4: return 3;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_5: return 4;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_6: return 5;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_7: return 6;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_8: return 7;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_9: return 8;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_10: return 9;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_11: return 10;
	case inventory_gui_intent_type::SPECIAL_ACTION_BUTTON_12: return 11;
	default: return -1;
	}
}

game_gui_system::pending_entropy_type game_gui_system::get_and_clear_pending_events() {
	auto out = pending;
	pending.clear();
	return out;
}

slot_button& game_gui_system::get_slot_button(const inventory_slot_id id) {
	return slot_buttons[id];
}

const slot_button& game_gui_system::get_slot_button(const inventory_slot_id id) const {
	return slot_buttons.at(id);
}

item_button& game_gui_system::get_item_button(const entity_id id) {
	return item_buttons[id];
}

const item_button& game_gui_system::get_item_button(const entity_id id) const {
	return item_buttons.at(id);
}

void game_gui_system::queue_transfer(const entity_id& subject, const item_slot_transfer_request req) {
	(void)subject;
	ensure(req.target_slot.is_valid());
	pending.transfer = req;
}

void game_gui_system::queue_wielding(const entity_id& subject, const wielding_setup& wielding) {
	(void)subject;
	pending.wield = wielding;
}

void game_gui_system::queue_swap_hotbar_buttons(const entity_id a, const entity_id b) {
	pending.swap_hotbar_buttons.a = a;
	pending.swap_hotbar_buttons.b = b;

	/*
		Optimistic local swap so that high-refresh render frames between now
		and the next logical step see the new order immediately. The next
		rebuild_hotbar will overwrite this with the cosmos-driven order which,
		by the time it runs, has the solver-applied swap baked in.
	*/
	auto& hotbar = local_gui.hotbar_buttons;

	for (auto& slot_a : hotbar) {
		if (slot_a.assigned == a) {
			for (auto& slot_b : hotbar) {
				if (slot_b.assigned == b) {
					std::swap(slot_a.assigned, slot_b.assigned);
					return;
				}
			}
			return;
		}
	}
}
	
bool game_gui_system::control_gui_world(
	const game_gui_context context,
	const augs::event::change change
) {
	const auto root_entity = context.get_subject_entity();

	ensure(root_entity.alive());

	const auto& cosm = root_entity.get_cosmos();
	auto& element = context.get_character_gui();

	if (root_entity.has<components::item_slot_transfers>()) {
		auto handle_drag_of = [&](const auto& item_entity) {
			auto& dragged_charges = element.dragged_charges;

			if (change.was_pressed(augs::event::keys::key::RMOUSE)) {
				if (world.held_rect_is_dragged) {
					auto access = allocate_new_entity_access();

					item_slot_transfer_request drop_some;
					drop_some.item = item_entity;
					drop_some.params.set_specified_quantity(access, dragged_charges);

					queue_transfer(root_entity, drop_some);
					world.unhover_and_undrag(context);
					return true;
				}
			}

			if (change.msg == augs::event::message::wheel) {
				const auto item = item_entity.template get<components::item>();

				const auto delta = change.data.scroll.amount;

				dragged_charges += delta;

				if (dragged_charges <= 0) {
					dragged_charges = item.get_charges() + dragged_charges;
				}
				if (dragged_charges > item.get_charges()) {
					dragged_charges = dragged_charges - item.get_charges();
				}

				return true;
			}

			return false;
		};

		if (const auto held_rect = context.get_if<item_button_in_item>(world.rect_held_by_lmb)) {
			const auto& item_entity = cosm[held_rect.get_location().item_id];

			if (handle_drag_of(item_entity)) {
				return true;
			}
		}

		if (const auto held_rect = context.get_if<hotbar_button_in_character_gui>(world.rect_held_by_lmb)) {
			if (handle_drag_of(held_rect->get_assigned_entity(root_entity))) {
				return true;
			}
		}
	}

	const auto gui_events = world.consume_raw_input_and_generate_gui_events(
		context,
		change
	);

	world.respond_to_events(
		context,
		gui_events
	);

	if (!gui_events.empty()) {
		return true;
	}

	return false;
}

void game_gui_system::control_hotbar_and_action_button(
	const const_entity_handle gui_entity,
	const inventory_gui_intent intent,
	const game_gui_input_settings& input
) {
	ensure(gui_entity.alive());

	if (/* not_applicable */ 
		!gui_entity.has<components::item_slot_transfers>()
		|| !gui_entity.has<components::sentience>()
	) {
		return;
	}

	auto& gui = get_character_gui();

	{
		auto r = intent;

		if (r.was_pressed()) {
			auto request_setup = [&](auto new_setup, const auto& current_setup) {
				if (::is_currently_reloading(gui_entity)) {
					new_setup = wielding_setup::bare_hands();
				}

				queue_wielding(gui_entity, new_setup);
				gui.save_as_last_setup(current_setup);
			};

			if (r.intent == inventory_gui_intent_type::HOLSTER) {
				const auto current_setup = wielding_setup::from_current(gui_entity);

				if (current_setup != wielding_setup::bare_hands()) {
					request_setup(wielding_setup::bare_hands(), current_setup);
				}

				return;
			}
		}
	}

	{
		const auto& i = intent;

		if (const auto hotbar_button_index = to_hotbar_index(i.intent);
			hotbar_button_index >= 0 && 
			static_cast<std::size_t>(hotbar_button_index) < gui.hotbar_buttons.size()
		) {
			auto& currently_held_index = gui.currently_held_hotbar_button_index;

			if (currently_held_index > -1) {
				if (gui.hotbar_buttons[currently_held_index].get_assigned_entity(gui_entity).dead()) {
					/* Clear currently held index because nothing is assigned already */
					currently_held_index = -1;
				}
			}

			if (gui.hotbar_buttons[hotbar_button_index].get_assigned_entity(gui_entity)) {
				/* Something is assigned to that button */
				if (i.was_pressed()) {
					const bool wants_dual_wield = currently_held_index > -1;

					if (wants_dual_wield) {
						const auto akimbo_setup = gui.get_setup_from_button_indices(
							gui_entity, 
							currently_held_index, 
							hotbar_button_index
						);

						queue_wielding(gui_entity, akimbo_setup);

						currently_held_index = -1;
					}
					else {
						currently_held_index = hotbar_button_index;

						auto new_setup = gui.get_setup_from_button_indices(
							gui_entity, 
							hotbar_button_index
						);

						const auto current_setup = wielding_setup::from_current(gui_entity);
						bool skip_wield = false;
						bool skip_save = false;

						const auto& cosm = gui_entity.get_cosmos();

						if (new_setup.equivalent_to_or_switched(cosm, current_setup)) {
							auto is_nade_or_none = [&](auto h) {
								if (cosm[h].dead()) {
									return true;
								}

								return cosm[h].template has<components::hand_fuse>();
							};
							
							if (is_nade_or_none(new_setup.hand_selections[0]) && is_nade_or_none(new_setup.hand_selections[1])) {
								/* Wouldn't switch hands for grenades */
								skip_wield = true;
							}
							else {
								new_setup = current_setup;
								new_setup.switch_hands();
								skip_save = true;
							}
						}

						if (!skip_wield) {
							if (!skip_save) {
								gui.save_as_last_setup(current_setup);
							}

							queue_wielding(gui_entity, new_setup);
						}
					}
				}
				else {
					currently_held_index = -1;
				}
			}
			else {
				/* 
					Corner case: 

					What if we want to dual-wield two entities stacked into the same hotbar button?
					We can communicate this by choosing an empty slot as the second one.
				*/

				const bool wants_dual_wield = currently_held_index > -1;

				if (wants_dual_wield && i.was_pressed()) {
					int count = 0;
					gui.hotbar_buttons[currently_held_index].get_assigned_entity(gui_entity, &count);

					const bool held_is_stacked = count > 1;

					if (held_is_stacked) {
						const auto akimbo_setup = gui.get_setup_from_button_indices(
							gui_entity, 
							currently_held_index, 
							currently_held_index
						);

						queue_wielding(gui_entity, akimbo_setup);

						currently_held_index = -1;
					}
				}
			}
		}
		else if (
			const auto special_action_index = to_special_action_index(i.intent);
			special_action_index >= 0 && 
			static_cast<std::size_t>(special_action_index) < gui.action_buttons.size()
		) {
			auto& action_b = gui.action_buttons[special_action_index];
			action_b.detector.update_appearance(i.was_pressed() ? gui_event::ldown : gui_event::lup);

			if (i.was_pressed()) {
				auto index = int(special_action_index);
				auto id = action_button_in_character_gui{index};
				auto loc = make_dereferenced_location(&gui.action_buttons[index], id);
				auto bound_spell = action_button::get_bound_spell(gui_entity, loc);

				if (bound_spell.is_set()) {
					pending.cast_spell = bound_spell;
				}
			}
		}
		else if (i.intent == inventory_gui_intent_type::LAST_USED_WEAPON && i.was_pressed()) {
			auto wielding = gui.make_wielding_setup_for_last_hotbar_selection_setup(gui_entity, input);

			const bool reset_lefthanded_to_righthanded = true;

			if (reset_lefthanded_to_righthanded) {
				if (wielding.is_left_handed(gui_entity.get_cosmos())) {
					wielding.switch_hands();
				}
			}

			const auto current_setup = wielding_setup::from_current(gui_entity);

			const auto& cosm = gui_entity.get_cosmos();

			if (current_setup.equivalent_to_or_switched(cosm, wielding)) {
				queue_wielding(gui_entity, wielding_setup::bare_hands());
			}
			else {
				queue_wielding(gui_entity, wielding);
			}

			//if (!wielding.same_as_in(gui_entity)) {
				//}
		}
	}
}

void game_gui_system::build_tree_data(const game_gui_context context) {
	if (context.get_subject_entity().dead()) {
		return;
	}

	if (!context.get_subject_entity().template has<components::item_slot_transfers>()) {
		return;
	}

	world.build_tree_data_into(context);
}

void game_gui_system::advance(
	const game_gui_context context,
	const augs::delta dt
) {
	const auto subject = context.get_subject_entity();

	ensure(subject.alive());

	const bool drop_armed = context.dependencies.game_gui.autodrop_holstered_armed_explosives;
	const bool drop_orphans = context.dependencies.game_gui.autodrop_magazines_of_dropped_weapons;

	if (drop_armed) {
		subject.for_each_contained_item_recursive(
			[&](const auto& typed_item) {
				typed_item.template dispatch_on_having_all<components::hand_fuse>(
					[&](const auto& expl) {
						const auto& fuse = expl.template get<components::hand_fuse>();

						if (fuse.armed() && fuse.slot_when_armed != expl.get_current_slot().get_type()) {
							LOG_NVPS(fuse.slot_when_armed, expl.get_current_slot().get_type());
							const auto impulse = subject.template get<invariants::item_slot_transfers>().standard_throw_impulse;
							auto request = item_slot_transfer_request::drop(typed_item, impulse);

							LOG("AUTO DROP");
							queue_transfer(subject, request);
						}
					}
				);
			}
		);
	}

	const auto recently_dropped_handle = subject.get_cosmos()[recently_dropped];

	if (logically_empty(pending.transfer) && recently_dropped_handle.alive() && drop_orphans) {
		/* TODO: FIX THIS IF WE EVER ALLOW SAME MAG TO TWO DIFFERENT WEAPONS */

		std::vector<item_flavour_id> needed_ammo_flavours;

		bool has_another_like_dropped = false;

		subject.for_each_contained_item_recursive(
			[&](const auto& typed_item) {
				if (entity_flavour_id(typed_item.get_flavour_id()) == recently_dropped_handle.get_flavour_id()) {
					has_another_like_dropped = true;
					return recursive_callback_result::ABORT;
				}

				return recursive_callback_result::CONTINUE_AND_RECURSE;
			}
		);

		if (!has_another_like_dropped) {
			{
				recently_dropped_handle.template dispatch_on_having_all<invariants::item>(
					[&](const auto& typed_item) {
						needed_ammo_flavours = ::calc_all_ammo_pieces_of(typed_item);
					}
				);
			}

			if (logically_set(needed_ammo_flavours)) {
				subject.for_each_contained_item_recursive(
					[&](const auto& typed_item) {
						if (::is_weapon_like(typed_item)) {
							return recursive_callback_result::CONTINUE_DONT_RECURSE;
						}

						if (!::is_ammo_piece_like(typed_item)) {
							return recursive_callback_result::CONTINUE_AND_RECURSE;
						}

						if (found_in(needed_ammo_flavours, item_flavour_id(typed_item.get_flavour_id()))) {
							queue_transfer(subject, item_slot_transfer_request::drop(typed_item));
						}

						return recursive_callback_result::CONTINUE_DONT_RECURSE;
					}
				);
			}
		}

		if (logically_empty(pending.transfer)) {
			recently_dropped = {};
		}
	}

	world.advance_elements(context, dt);
}

void game_gui_system::rebuild_layouts(
	const game_gui_context context
) {
	const auto scale = ImGui::GetTextLineHeight() / 22.0f;

	const auto root_entity = context.get_subject_entity();

	ensure(root_entity.alive());

	const auto& necessarys = context.get_necessary_images();
	const auto& image_defs = context.get_image_metas();
	auto& element = context.get_character_gui();

	const bool hide_unassigned_hotbar_buttons = context.get_hotbar_settings().hide_unassigned_hotbar_buttons;

	if (root_entity.has<components::item_slot_transfers>()) {
		const auto screen_size = context.get_screen_size();

		int max_hotbar_height = 55;

		{
			int total_width = 0;

			for (size_t i = 0; i < element.hotbar_buttons.size(); ++i) {
				const auto& hb = element.hotbar_buttons[i];

				if (hide_unassigned_hotbar_buttons) {
					if (const auto ent = hb.get_assigned_entity(root_entity); ent.dead()) {
						continue;
					}
				}

				const auto bbox = hb.get_bbox(necessarys, image_defs, root_entity);
				max_hotbar_height = std::max(max_hotbar_height, bbox.y);

				total_width += bbox.x;
			}

			const int left_rc_spacing = 2;
			const int right_rc_spacing = 1;

			int current_x = screen_size.x / 2 - total_width / 2 - left_rc_spacing;

			const auto set_rc = [&](auto& hb) {
				if (hide_unassigned_hotbar_buttons) {
					if (const auto ent = hb.get_assigned_entity(root_entity); ent.dead()) {
						hb.rc = {};

						return;
					}
				}

				const auto bbox = hb.get_bbox(necessarys, image_defs, root_entity);

				hb.rc = xywh(xywhi(current_x, screen_size.y - max_hotbar_height - 50 * scale, bbox.x + left_rc_spacing + right_rc_spacing, max_hotbar_height));

				current_x += bbox.x + left_rc_spacing + right_rc_spacing;
			};

			for (size_t i = 0; i < element.hotbar_buttons.size(); ++i) {
				set_rc(element.hotbar_buttons[i]);
			}

			if (total_width == 0) {
				max_hotbar_height = 0;
			}
		}

		{
			const auto action_button_size = context.get_necessary_images().at(assets::necessary_image_id::ACTION_BUTTON_BORDER).get_original_size();

			int used_action_buttons = 0;

			for (size_t i = 0; i < element.action_buttons.size(); ++i) {
				auto id = action_button_in_character_gui{int(i)};
				auto loc = make_dereferenced_location(&element.action_buttons[i], id);
				auto bound = action_button::get_bound_spell(context.get_subject_entity(), loc);

				if (bound.is_set()) {
					++used_action_buttons;
				}
			}

			auto total_width = static_cast<int>(used_action_buttons) * action_button_size.x;

			const int left_rc_spacing = 4;
			const int right_rc_spacing = 3;

			int current_x = screen_size.x / 2 - total_width / 2 - left_rc_spacing;

			const auto set_rc = [&](auto& hb) {
				const auto bbox = action_button_size;

				hb.rc = xywh(xywhi(
					current_x, screen_size.y - action_button_size.y - 9 - max_hotbar_height - 50 * scale, 
					bbox.x + left_rc_spacing + right_rc_spacing, 
					action_button_size.y));

				current_x += bbox.x + left_rc_spacing + right_rc_spacing;
			};

			for (size_t i = 0; i < element.action_buttons.size(); ++i) {
				auto id = action_button_in_character_gui{int(i)};
				auto loc = make_dereferenced_location(&element.action_buttons[i], id);
				auto bound = action_button::get_bound_spell(context.get_subject_entity(), loc);

				if (bound.is_set()) {
					set_rc(element.action_buttons[i]);
				}
			}
		}
	}

	world.rebuild_layouts(context);
}

void game_gui_system::standard_post_solve(
	const const_logic_step step,
	const const_entity_handle subject,
	const game_gui_post_solve_settings settings
) {
	auto migrate_nodes = [&](auto& migrated, const auto& key_map) {
		using M = remove_cref<decltype(migrated)>;

		M result;

		for (auto& it : migrated) {
			if (const auto new_id = mapped_or_nullptr(key_map, it.first)) {
				result.try_emplace(*new_id, std::move(it.second));
			}
			else {
				result.try_emplace(it.first, std::move(it.second));
			}
		}

		migrated = std::move(result);
	};

	for (const auto& changed : step.get_queue<messages::changed_identities_message>()) {
		const auto predictability = changed.predictable ? always_predictable_v : never_predictable_v;

		if (!predictability.should_play(settings.prediction)) {
			continue;
		}

		migrate_nodes(item_buttons, changed.changes);

		auto migrate_id = [&](auto& id) {
			if (const auto new_id = mapped_or_nullptr(changed.changes, id)) {
				id = *new_id;
			}
		};

		for (auto& s : local_gui.last_setup.hand_selections) {
			migrate_id(s);
		}
	}

	rebuild_hotbar(subject);
}

void game_gui_system::rebuild_hotbar(const const_entity_handle subject) {
	/*
		Stateless rebuild. Each logical step we sort the subject's weapon-like
		inventory by components::item::incoming_transfer_id ascending and lay
		the result out into hotbar buttons in order. Stackable items (grenades,
		melees) collapse: the slot for a given flavour is the earliest-acquired
		instance, the rest are reachable via flavour matching in get_assigned_entity.

		A manual hotbar swap (drag-and-drop) is implemented elsewhere by
		swapping the incoming_transfer_id of two items - the next rebuild then
		picks up the new order automatically. No flavour fallback, no migration,
		no stale-id healing - the hotbar cache is rebuilt from scratch every
		step, with rendering between steps reading the cached entity_ids directly.
	*/

	auto& hotbar = local_gui.hotbar_buttons;

	auto clear_all = [&]() {
		for (auto& slot : hotbar) {
			slot.assigned = {};
		}
	};

	if (subject.dead()) {
		clear_all();
		return;
	}

	const auto& cosm = subject.get_cosmos();
	const auto current_step = cosm.get_timestamp().step;
	const auto current_subject = subject.get_id();

	if (current_step == last_rebuilt_step
		&& current_subject == last_rebuilt_subject
	) {
		return;
	}

	auto is_interesting_for_hotbar = [](const auto item) {
		if (!is_weapon_like(item)) {
			return false;
		}

		if (is_like_plantable_bomb(item)) {
			return false;
		}

		const auto slot = item.get_current_slot();

		if (slot.alive() && slot.get_type() == slot_function::PERSONAL_DEPOSIT) {
			return false;
		}

		return true;
	};

	struct candidate {
		entity_id id;
		entity_flavour_id flavour;
		uint32_t order = 0;
		bool stackable = false;
	};

	std::array<candidate, 64> buf;
	std::size_t buf_n = 0;

	subject.for_each_contained_item_recursive(
		[&](const auto& item) {
			if (!is_interesting_for_hotbar(item)) {
				return recursive_callback_result::CONTINUE_AND_RECURSE;
			}

			if (buf_n >= buf.size()) {
				return recursive_callback_result::CONTINUE_AND_RECURSE;
			}

			buf[buf_n++] = candidate {
				item.get_id(),
				item.get_flavour_id(),
				item.template get<components::item>().get_incoming_transfer_id(),
				::is_stackable_in_hotbar(item)
			};

			return recursive_callback_result::CONTINUE_AND_RECURSE;
		}
	);

	std::sort(buf.begin(), buf.begin() + buf_n, [](const candidate& a, const candidate& b) {
		return a.order < b.order;
	});

	clear_all();

	/*
		Walk sorted candidates; for stackables, skip any subsequent instance of
		a flavour already represented by an earlier (lower-id) one.
	*/
	std::size_t next_slot = 0;

	for (std::size_t i = 0; i < buf_n && next_slot < hotbar.size(); ++i) {
		const auto& c = buf[i];

		if (c.stackable) {
			bool already_represented = false;

			for (std::size_t k = 0; k < next_slot; ++k) {
				const auto stored = cosm[hotbar[k].assigned];

				if (stored.alive() && stored.get_flavour_id() == c.flavour) {
					already_represented = true;
					break;
				}
			}

			if (already_represented) {
				continue;
			}
		}

		hotbar[next_slot++].assigned = c.id;
	}

	last_rebuilt_step = current_step;
	last_rebuilt_subject = current_subject;
}
