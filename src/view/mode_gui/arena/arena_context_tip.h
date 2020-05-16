#pragma once
#include "game/detail/buy_area_in_range.h"
#include "game/detail/bombsite_in_range.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/detail/start_defusing_nearby_bomb.h"

template <class E, class M, class I>
inline void draw_context_tip(
	const M& typed_mode,
	const I& mode_input,
	const config_lua_table& config,
	const augs::drawer_with_default out,
	const vec2i screen_size,
	const augs::baked_font& font,
	const E& viewed_character,
	const faction_type current_faction,
	const bool team_choice_opened,
	const bool buy_menu_opened,
	const bool is_cursor_released
) {
	using namespace augs::gui::text;
	using namespace augs::event;

	const auto& cosm = mode_input.cosm;

	auto colored = [&](const auto& text, const auto& c) {
		const auto text_style = style(font, c);

		return formatted_string(text, text_style);
	};

	const auto& maybe_settings = config.arena_mode_gui.context_tip_settings;

	if (!maybe_settings.is_enabled) {
		return;
	}

	const auto& settings = maybe_settings.value;

	auto format_key = [&](const auto& m, const auto key) {
		const auto found_k = key_or_default(m, key);

		if (found_k == keys::key()) {
			return colored(" (UNASSIGNED) ", settings.bound_key_color);
		}

		const auto key_str = [&]() {
			const auto shortened = key_to_string_shortened(found_k);

			if (shortened.length() == 1) {
				return "\"" + shortened + "\"";
			}

			const auto alt_char = key_to_alternative_char_representation(found_k);

			if (alt_char.empty()) {
				return shortened;
			}

			return alt_char + " (" + shortened + ")" ;
		}();

		return colored(" " + key_str + " ", settings.bound_key_color);
	};

	const auto final_pos = vec2i(screen_size.x / 2, settings.tip_offset_mult * screen_size.y);
	const auto line_height = font.metrics.get_height();

	auto pen = final_pos;

	auto do_line = [&](const auto& with_text) {
		print_stroked(
			out,
			pen,
			with_text,
			{ augs::ralign::CX, augs::ralign::CY }
		);

		pen.y += line_height;
	};

	const auto total_text = [&]() {
		formatted_string total_text;

		auto break_line = [&]() {
			do_line(total_text);
			total_text.clear();
		};

		auto get_key_str = [&](auto h) {
			using H = decltype(h);

			if constexpr(std::is_same_v<H, general_gui_intent_type>) {
				return format_key(config.general_gui_controls, h);
			}
			else if constexpr(std::is_same_v<H, game_intent_type>) {
				return format_key(config.game_controls, h);
			}
			else if constexpr(std::is_same_v<H, inventory_gui_intent_type>) {
				return format_key(config.inventory_gui_controls, h);
			}
			else {
				static_assert(always_false_v<H>);
			}
		};

		auto hotkey = [&](auto h) {
			total_text += get_key_str(h);
		};

		auto text = [&](const auto& str) {
			total_text += colored(str, settings.tip_text_color);
		};

		if (current_faction == faction_type::SPECTATOR) {
			if (!team_choice_opened) {
				text("You are a Spectator. Press");
				hotkey(general_gui_intent_type::CHOOSE_TEAM);
				text("to change teams.");
			}

			return total_text;
		}

		if (viewed_character.dead()) {
			return total_text;
		}

		if (!sentient_and_conscious(viewed_character)) {
			return total_text;
		}

		if (is_cursor_released) {
			text("You have released the cursor and can now interact with GUI.");
			break_line();
			text("To control the crosshair, press"); hotkey(general_gui_intent_type::TOGGLE_MOUSE_CURSOR); text("again.");

			return total_text;
		}

		if (::buy_area_in_range(viewed_character)) {
			if (!buy_menu_opened) {
				if (typed_mode.get_buy_seconds_left(mode_input) <= 0.f) {
					text("It is too late to buy items.");
					return total_text;
				}

				text("Press");
				hotkey(general_gui_intent_type::BUY_MENU);
				text("to buy items.");
			}

			return total_text;
		}

		bool is_bomb_in_hand = false;
		bool bomb_being_armed = false;
		std::size_t bomb_hand_index;

		const auto bomb = [&]() -> const_entity_handle {
			entity_id result;

			viewed_character.for_each_contained_item_recursive(
				[&](const auto& typed_item) {
					if (const auto hand_fuse = typed_item.template find<invariants::hand_fuse>()) {
						if (hand_fuse->is_like_plantable_bomb()) {
							result = typed_item;

							const auto slot = typed_item.get_current_slot();
							is_bomb_in_hand = slot.is_hand_slot();
							bomb_hand_index = slot.get_hand_index();

							if (const auto hand_fuse_comp = typed_item.template find<components::hand_fuse>()) {
								if (hand_fuse_comp->when_started_arming.was_set()) {
									bomb_being_armed = true;
								}
							}
						}
					}
				}
			);

			return cosm[result];
		}();

		if (bomb) {
			const auto participants = typed_mode.calc_participating_factions(mode_input);

			if (participants.defusing == current_faction) {
				text("You've stolen the bomb! Escape!");
				break_line();
				text("Careful, enemies know where you go with the bomb.");
				return total_text;
			}
		}

		if (bomb_being_armed) {
			text("Stay still while planting the bomb!");

			return total_text;
		}

		if (is_bomb_in_hand) {
			if (::bombsite_in_range(bomb)) {
				text("Press and hold");
				hotkey(bomb_hand_index == 0 ? game_intent_type::SHOOT : game_intent_type::SHOOT_SECONDARY);
				text("to plant the bomb.");

				return total_text;
			}
			else {
				text("You cannot plant the bomb here.");
				break_line();
				text("Find the bombsite!");
				return total_text;
			}
		}

		if (::bombsite_in_range_of_entity(viewed_character)) {
			if (bomb) {
				text("Press");
				hotkey(game_intent_type::WIELD_BOMB);
				text("to pull out the bomb.");

				return total_text;
			}
		}

		const auto defuse_request = ::query_defusing_nearby_bomb(viewed_character); 

		if (defuse_request.defusing_already) {
			text("Stay still while defusing.");

			if (viewed_character.get_wielded_items().size() > 0) {
				break_line();
				text("Hide items with");
				hotkey(inventory_gui_intent_type::HOLSTER);
				text("or drop them with");
				hotkey(game_intent_type::DROP);
				text("to defuse faster!");
			}

			return total_text;
		}

		if (defuse_request.success()) {
			text("Press");
			hotkey(game_intent_type::INTERACT);
			text("to defuse the bomb.");

			return total_text;
		}

		(void)viewed_character;
		return total_text;
	}();

	do_line(total_text);
}
