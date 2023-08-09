#pragma once
#include "game/detail/buy_area_in_range.h"
#include "game/detail/bombsite_in_range.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/detail/use_interaction_logic.h"
#include "view/mode_gui/arena/render_text_with_hotkeys.hpp"
#include "view/mode_gui/arena/on_first_touching_portal.hpp"

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
	const auto bad_color = rgba(255, 50, 50, 255);

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
			return colored("(UNASSIGNED)", settings.bound_key_color);
		}

		const auto key_str = [&]() {
			const auto shortened = key_to_string(found_k);

			if (shortened.length() == 1) {
				return "\"" + shortened + "\"";
			}

			const auto alt_char = key_to_alternative_char_representation(found_k);

			if (alt_char.empty()) {
				return shortened;
			}

			return alt_char + " (" + shortened + ")" ;
		}();

		return colored(key_str, settings.bound_key_color);
	};

	const auto final_pos = vec2i(screen_size.x / 2, settings.tip_offset_mult * screen_size.y);
	const auto line_height = font.metrics.get_height();
	const auto& cfg = config.arena_mode_gui.scoreboard_settings;
	auto border_color = cfg.border_color;

	std::vector<formatted_string> lines;

	auto do_line = [&](auto with_text) {
		lines.emplace_back(std::move(with_text));
	};

	auto draw_everything = [&]() {
		vec2i total_bbox = {0, 0};

		bool any_text = false;

		for (const auto& line_text : lines) {
			if (!line_text.empty()) {
				any_text = true;
			}

			const auto bbox = get_text_bbox(line_text);

			total_bbox.x = std::max(total_bbox.x, bbox.x);
			total_bbox.y += bbox.y;
		}

		if (!any_text) {
			return;
		}

		const auto window_padding = vec2i(32, 16);

		const auto popup_lt = vec2(final_pos.x - total_bbox.x / 2, final_pos.y - line_height / 2);
		const auto window_bg_rect = ltrb(popup_lt, total_bbox).expand_from_center(window_padding);

		out.aabb_with_border(window_bg_rect, cfg.background_color, border_color);

		auto pen = final_pos;

		for (const auto& line_text : lines) {
			print_stroked(
				out,
				pen,
				line_text,
				{ augs::ralign::CX, augs::ralign::CY }
			);

			pen.y += line_height;
		}
	};

	const auto total_text = [&]() -> formatted_string {
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

		auto text_colored = [&](const auto& str, auto col) {
			total_text += colored(str, col);
		};

		(void)text_colored;

		auto do_text = [&](const auto& input) {
			render_text_with_hotkeys(input, text, break_line, hotkey);

			return total_text;
		};

		if (current_faction == faction_type::SPECTATOR) {
			if (!team_choice_opened) {
				return do_text("You are a Spectator. Press {CHOOSE_TEAM} to change teams.");
			}

			return {};
		}

		if (viewed_character.dead()) {
			return {};
		}

		if (!sentient_and_conscious(viewed_character)) {
			return {};
		}

		if (is_cursor_released) {
			return do_text("You have released the cursor and can now interact with GUI.\nTo control the crosshair, press {TOGGLE_MOUSE_CURSOR} again.");
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

		if constexpr(std::is_same_v<M, arena_mode>) {
			if (bomb) {
				const auto participants = typed_mode.calc_participating_factions(mode_input);

				if (participants.defusing == current_faction) {
					return do_text("You've stolen the bomb! Escape!\nCareful, enemies know where you go with the bomb.");
				}
			}
		}
		else {
			if (bomb) {
				if (current_faction == faction_type::METROPOLIS) {
					return do_text("You've stolen the bomb! Escape!\nCareful, enemies always see the bomb's location!");
				}
			}
		}

		if (bomb_being_armed) {
			return do_text("Stay still while planting the bomb!");
		}

		if (is_bomb_in_hand) {
			if (::bombsite_in_range(bomb)) {
				if (bomb_hand_index == 0) {
					return do_text("Press and hold {SHOOT} to plant the bomb.");
				}
				else {
					return do_text("Press and hold {SHOOT_SECONDARY} to plant the bomb.");
				}
			}
			else {
				return do_text("You cannot plant the bomb here.\nFind the bombsite! It's marked with red lines.");
			}
		}

		if (::bombsite_in_range_of_entity(viewed_character)) {
			if (bomb) {
				return do_text("Press {WIELD_BOMB} to pull out the bomb.");
			}
		}

		auto interaction_tip = [&](const auto& typed_interaction) {
			using T = remove_cref<decltype(typed_interaction)>;

			if constexpr(std::is_same_v<T, item_pickup>) {
				const auto item_handle = cosm[typed_interaction.item];
				const auto pickup_slot = viewed_character.find_pickup_target_slot_for(item_handle, { slot_finding_opt::OMIT_MOUNTED_SLOTS });

				if (pickup_slot.alive()) {
					text("Press ");
					hotkey(game_intent_type::INTERACT);
					text(" to pick up ");

					const auto& item_name = item_handle.get_name();
					total_text += colored(item_name, settings.item_name_color);

					text(".");
				}
				else {
					border_color = bad_color;

					text("Inventory is ");
					text_colored("full", bad_color);
					text(".");
					break_line();
					text("Buy a ");
					text_colored("Backpack", settings.bound_key_color);
					text(" to carry a lot of items.");
				}
			}
			else if constexpr(std::is_same_v<T, bomb_defuse_interaction>) {
				if (typed_interaction.is_in_progress()) {
					text("Stay still while defusing.");

					if (viewed_character.get_wielded_items().size() > 0) {
						break_line();
						text("Hide items with ");
						hotkey(inventory_gui_intent_type::HOLSTER);
						text(" or drop them with ");
						hotkey(game_intent_type::DROP);
						text(" to defuse faster!");
					}
				}

				if (typed_interaction.can_begin_interaction()) {
					text("Press ");
					hotkey(game_intent_type::INTERACT);
					text(" to defuse the bomb.");
				}
			}
		};

		if (const auto potential_interaction = ::query_use_interaction(viewed_character)) {
			std::visit(interaction_tip, *potential_interaction);

			return total_text;
		}

		if constexpr(!std::is_same_v<M, test_mode>) {
			if (mode_input.rules.has_economy()) {
				if (::buy_area_in_range(viewed_character) && !buy_menu_opened) {
					if (typed_mode.get_buy_seconds_left(mode_input) <= 0.f) {
						return do_text("It is too late to buy items.");
					}

					return do_text("Press {BUY_MENU} to buy items.");
				}
			}
		}

		on_first_touching_portal(
			viewed_character,
			[&](const auto& portal) {
				if (const auto details = portal.template find<invariants::text_details>()) {
					::render_text_with_hotkeys(details->description, text, break_line, hotkey);
				}
			}
		);

		(void)viewed_character;
		return total_text;
	}();

	do_line(total_text);

	draw_everything();
}
