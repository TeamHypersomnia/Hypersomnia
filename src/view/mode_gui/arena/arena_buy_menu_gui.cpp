#include "augs/templates/logically_empty.h"
#include "augs/string/format_enum.h"
#include "augs/templates/enum_introspect.h"
#include "view/mode_gui/arena/arena_buy_menu_gui.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "view/viewables/images_in_atlas_map.h"
#include "application/config_lua_table.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/cosmos/for_each_entity.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "application/app_intent_type.h"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/organization/special_flavour_id_types.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "augs/gui/formatted_string.h"
#include "view/mode_gui/arena/arena_mode_gui_settings.h"
#include "game/detail/visible_entities.hpp"
#include "game/detail/buy_area_in_range.h"

#include "game/detail/flavour_presentation.h"

void game_image_with_attachments(
	const ltrb precalculated_aabb,
	const vec2 image_padding,
	const images_in_atlas_map& images_in_atlas,
	const cosmos& cosm,
	const item_flavour_id& flavour
) {
	using namespace augs::imgui;

	const auto total_size = precalculated_aabb.get_size(); 
	const auto lt_offset = -precalculated_aabb.left_top();

	auto get_entry = [&](const auto id) {
		return images_in_atlas.find_or(id).diffuse;
	};

	auto draw = [&](
		const auto& attachment_image,
		const auto& attachment_offset,
		bool
	) {
		const auto& entry = get_entry(attachment_image);
		const auto size = entry.get_original_size();

		const auto final_pos = image_padding + lt_offset + attachment_offset.pos - size / 2;
		game_image(entry, size, white, final_pos, augs::imgui_atlas_type::GAME, attachment_offset.rotation);
	};

	presentational_with_attachments(
		draw,
		cosm,
		flavour
	);

	invisible_button("invisible_gmimg", total_size + image_padding);
}

bool should_close_after_purchase(buy_menu_type b) {
	switch (b) {
		case buy_menu_type::TOOLS: return false;
		case buy_menu_type::SPELLS: return false;
		case buy_menu_type::MELEE: return false;
		case buy_menu_type::GRENADES: return false;
		default: return true;
	}
}

static auto make_hotkey_map() {
	using namespace augs::event::keys;

	arena_buy_menu_hotkey_map m;
	m[key::_1] = buy_menu_type::PISTOLS;
	m[key::_2] = buy_menu_type::RIFLES;
	m[key::_3] = buy_menu_type::MELEE;
	m[key::_4] = buy_menu_type::SUBMACHINE_GUNS;
	m[key::_5] = buy_menu_type::HEAVY_GUNS;
	m[key::_6] = buy_menu_type::SHOTGUNS;
	m[key::_7] = buy_menu_type::GRENADES;
	m[key::_8] = buy_menu_type::SPELLS;
	m[key::_9] = buy_menu_type::ARMORS;
	m[key::_0] = buy_menu_type::TOOLS;

	return m;
}

void arena_buy_menu_gui::hide() {
	show = false;
	current_menu = buy_menu_type::MAIN;
}

bool arena_buy_menu_gui::escape() {
	if (!show) {
		return false;
	}

	if (current_menu != buy_menu_type::MAIN) {
		current_menu = buy_menu_type::MAIN;
		return true;
	}

	hide();
	return true;
}

bool arena_buy_menu_gui::control(general_gui_intent_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (const auto it = mapped_or_nullptr(in.controls, key)) {
			if (*it == general_gui_intent_type::BUY_MENU) {
				key_opened = key;

				if (show) {
					hide();
				}
				else {
					show = true;
				}

				return true;
			}
		}
	}

	if (!show) {
		return false;
	}
	
	static const auto m = make_hotkey_map();

	const auto& c = in.common_input_state;

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (current_menu == buy_menu_type::MAIN) {
			if (!c.keys[key::LSHIFT]) {
				if (const auto it = mapped_or_nullptr(m, key)) {
					current_menu = *it;
					return true;
				}
			}
		}

		if (const auto n = get_number(key)) {
			if (c.keys[key::LSHIFT]) {
				requested_replenishables.push_back(*n);
				selected_replenishables.emplace(*n);
			}
			else {
				requested_weapons.push_back(*n);
				selected_weapons.emplace(*n);
			}

			return true;
		}
	}

	if (ch == key_change::RELEASED) {
		const auto key = in.e.get_key();

		if (const auto n = get_number(key)) {
			selected_weapons.erase(*n);
			selected_replenishables.erase(*n);
			return true;
		}
	}

	return false;
}

using input_type = arena_buy_menu_gui::input;
using result_type = mode_player_entropy;

result_type arena_buy_menu_gui::perform_imgui(const input_type in) {
	using namespace augs::imgui;
	(void)in;

	auto next_requested_menu = current_menu;
	const auto& subject = in.subject;
	const auto& cosm = subject.get_cosmos();
	const auto money_color = in.money_indicator_color;

	if (subject.dead()) {
		return {};
	}

	if (!buy_area_in_range(subject)) {
		hide();
		return {};
	}

	if (!show) {
		return {};
	}

	center_next_window(vec2::square(0.9f), ImGuiCond_Once);

	const auto window_name = "Buy menu";
	auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar);

	centered_text(window_name);

	if (const auto& entry = in.images_in_atlas.find_or(in.money_icon).diffuse; entry.exists()) {
		game_image_button("#moneyicon", entry);
		ImGui::SameLine();
	}

	text_color(typesafe_sprintf("%x$", in.available_money), money_color);

	const auto& spells = cosm.get_common_significant().spells;

	auto price_of = [&](const auto& of) {
		return *::find_price_of(cosm, of);
	};

	enum class button_type {
		REPLENISHABLE,
		BUYABLE_WEAPON
	};

	auto draw_owned_weapon = [&](const item_flavour_id& f_id) {
		if (const auto& entry = in.images_in_atlas.find_or(::image_of(cosm, f_id)).diffuse; entry.exists()) {
			const auto image_padding = vec2(0, 4);
			const auto size = vec2(entry.get_original_size());
			game_image(entry, size, white, image_padding);
			invisible_button("invisible_owned", size + image_padding);
		}
	};

	result_type result;

	if (requested_special.has_value()) {
		result = *requested_special;
		requested_special = std::nullopt;
		return result;
	}

	auto set_result = [&](const auto& r, const bool maybe_close = true) {
		result = r;

		if (maybe_close && ::should_close_after_purchase(current_menu)) {
			next_requested_menu = buy_menu_type::MAIN;
		}
	};

	const auto& item_spacing = ImGui::GetStyle().ItemSpacing;

	const auto line_h = ImGui::GetTextLineHeight();
	const auto standard_button_h = line_h * 2 + item_spacing.y + 1;

	auto darken_selectables = []() {
		const auto hover_col = rgba(ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
		const auto active_col = rgba(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);

		return std::make_tuple(
			scoped_style_color(ImGuiCol_HeaderHovered, rgba(hover_col).multiply_rgb(1 / 2.1f)),
			scoped_style_color(ImGuiCol_HeaderActive, rgba(active_col).multiply_rgb(1 / 1.8f))
		);
	};

	enum class owning_type {
		UNOWNED,
		OWNED,
		OWNED_OF_THE_SAME_TYPE
	};

	auto general_purchase_button = [&](
		auto purchasable_graphic_size,
		auto draw_purchasable_callback,
		const auto& flavour_or_spell, 
		const auto& selected,
		const owning_type owning,
		const std::optional<int> num_carryable,
		const int index,
		const std::string& additional_id,
		const auto& hotkey_text,
		auto&& name_callback,
		auto&& price_callback
	) {
		const auto size = static_cast<vec2>(purchasable_graphic_size);
		const auto price = price_of(flavour_or_spell);

		if (price == 0) {
			return false;
		}

		const auto local_pos = ImGui::GetCursorPos();
		const auto button_h = std::max(size.y, line_h * 2) + item_spacing.y;

		const auto num_affordable = in.available_money / price;

		const bool is_disabled = num_affordable == 0 || (num_carryable && *num_carryable == 0);

		bool result = false;

		auto darkened = darken_selectables();

		const auto label_id = typesafe_sprintf("##%xr%x", index, additional_id);

		const bool active = found_in(selected, index);

		if (owning == owning_type::OWNED) {
			const auto col = active ? in.settings.already_owns_active_bg : in.settings.already_owns_bg;
			const auto selectable_size = ImVec2(ImGui::GetContentRegionAvail().x, 2 * item_spacing.y + button_h);
			rect_filled(selectable_size, col, vec2(0, -item_spacing.y + 1));
		}
		else if (owning == owning_type::OWNED_OF_THE_SAME_TYPE) {
			const auto col = active ? in.settings.already_owns_other_type_active_bg : in.settings.already_owns_other_type_bg;
			const auto selectable_size = ImVec2(ImGui::GetContentRegionAvail().x, 2 * item_spacing.y + button_h);
			rect_filled(selectable_size, col, vec2(0, -item_spacing.y + 1));
		}
		else if (is_disabled) {
			const auto col = active ? in.settings.disabled_active_bg : in.settings.disabled_bg;
			const auto selectable_size = ImVec2(ImGui::GetContentRegionAvail().x, 2 * item_spacing.y + button_h);
			rect_filled(selectable_size, col, vec2(0, -item_spacing.y + 1));
		}
		else {
			const auto selectable_size = ImVec2(0, 1 + button_h);
			result = ImGui::Selectable(label_id.c_str(), active, ImGuiSelectableFlags_None, selectable_size);
		}

		ImGui::SetCursorPos(local_pos);

		text_disabled(hotkey_text);
		ImGui::SameLine();

		draw_purchasable_callback();

		ImGui::SameLine();

		const auto x = ImGui::GetCursorPosX();
		name_callback();

		const auto prev_y = ImGui::GetCursorPosY() + line_h;
		ImGui::SetCursorPosY(prev_y);
		ImGui::SetCursorPosX(x);

		if (owning == owning_type::OWNED) {
			text("Owned already");
		}
		else if (owning == owning_type::OWNED_OF_THE_SAME_TYPE) {
			text("Owned other item of the same type");
		}
		else {
			text_color(typesafe_sprintf("%x$", price), money_color);
			ImGui::SameLine();
			ImGui::SetCursorPosY(prev_y);
			price_callback(num_affordable);
		}

		ImGui::SetCursorPosY(local_pos.y + button_h + item_spacing.y);

		return result;
	};

	auto purchase_spell_button = [&](
		const spell_id& s_id, 
		const int index
	) {
		const auto& selected = selected_weapons;

		const auto additional_id = "s";
		const auto hotkey_text = typesafe_sprintf("(%x)", index);

		static const augs::baked_font dummy; 
		/* In case we make the font statically allocated one day */
		static_assert(sizeof(dummy) < 1000);

		const bool learnt = subject.template get<components::sentience>().is_learnt(s_id);

		const auto image = ::get_spell_image(cosm, s_id);
		const auto& entry = in.images_in_atlas.find_or(image).diffuse;

		if (!entry.exists()) {
			return false;
		}

		const auto size = entry.get_original_size();

		auto draw_callback = [&]() {
			const auto image_padding = vec2(0, 4);

			game_image(entry, size, white, image_padding);
			invisible_button("invisible_spell", size + image_padding);
		};

		return general_purchase_button(
			size,
			draw_callback,
			s_id,
			selected,
			learnt ? owning_type::OWNED : owning_type::UNOWNED,
			1,
			index,
			additional_id,
			hotkey_text,
			[&]() {
				::on_spell(cosm, s_id, [&](const auto& spell_data) {
					const auto col = spell_data.appearance.name_color;
					text_color(spell_data.appearance.name, col);
					ImGui::SameLine();
				});
			},
			[&](auto num_affordable) {
				num_affordable = std::min(num_affordable, 1);

				auto& requested = requested_weapons;

				if (requested.size() > 0) {
					if (requested.front() == index) {
						requested.erase(requested.begin());
						if (num_affordable > 0) {
							set_result(s_id);
						}
					}
				}

				::on_spell(cosm, s_id,  [&](const auto& spell_data) {
					(void)spell_data;
					const bool show_how_many_spells = false;

					if (!show_how_many_spells) {
						return;
					}

					const auto prev_y = ImGui::GetCursorPosY();
					text_disabled("(Can buy");
					ImGui::SameLine();
					ImGui::SetCursorPosY(prev_y);
					text_color(typesafe_sprintf("%x", num_affordable), num_affordable == 0 ? red : money_color);
					ImGui::SameLine();
					ImGui::SetCursorPosY(prev_y);
					text_disabled("more)");
				});
			}
		);
	};

	auto purchase_item_button = [&](
		const auto& f_id, 
		const button_type b, 
		const int index
	) {
		if (!is_alive(cosm, f_id)) {
			return false;
		}
		const auto& selected = b == button_type::REPLENISHABLE ? selected_replenishables : selected_weapons;

		auto buy_opts = ::get_buy_slot_opts();

		if (b == button_type::REPLENISHABLE) {
			/* Don't count hands as a space for replenishables, e.g. mags */
			erase_element(buy_opts, candidate_holster_type::HANDS);
		}

		const auto num_carryable = num_carryable_pieces(
			subject,
			buy_opts, 
			f_id
		);

		const auto additional_id = std::to_string(int(b));

		const bool is_replenishable = b == button_type::REPLENISHABLE;
		const auto hotkey_text = typesafe_sprintf(is_replenishable ? "(Shift+%x)" : "(%x)", index);

		const auto this_owning_category = ::calc_once_owning_category(cosm, f_id);

		const auto num_owned = subject.count_contained(f_id);
		const auto num_owned_of_same_once_category = subject.count_contained([&](const auto& typed_item) {
			if (this_owning_category == ::calc_once_owning_category(cosm, typed_item.get_flavour_id())) {
				return true;
			}

			return false;
		});

		const auto owned_status = [&]() {
			if (this_owning_category == once_owning_category::NONE) {
				return owning_type::UNOWNED;
			}

			if (num_owned >= 1) {
				return owning_type::OWNED;
			}

			return num_owned_of_same_once_category >= 1 ? owning_type::OWNED_OF_THE_SAME_TYPE : owning_type::UNOWNED;
		}();

		const auto aabb = ::aabb_of_game_image_with_attachments(
			in.images_in_atlas,
			cosm,
			f_id
		);

		if (!aabb.good()) {
			return false;
		}

		auto draw_callback = [&]() {
			const auto image_padding = vec2(0, 4);

			::game_image_with_attachments(
				aabb,
				image_padding,
				in.images_in_atlas,
				cosm,
				f_id
			);
		};

		return general_purchase_button(
			aabb.get_size(),
			draw_callback,
			f_id,
			selected,
			owned_status,
			num_carryable,
			index,
			additional_id,
			hotkey_text,
			[&]() {
				cosm.on_flavour(f_id, [&](const auto& typed_flavour) {
					text(typed_flavour.get_name());
					ImGui::SameLine();

					if (in.give_extra_mags_on_first_purchase && b == button_type::BUYABLE_WEAPON) {
						const auto& item_def = typed_flavour.template get<invariants::item>();
						const auto& g = item_def.gratis_ammo_pieces_with_first;

						if (g > 0) {
							if (!found_in(in.done_purchases, f_id)) {
								text_disabled("(+");
								ImGui::SameLine();
								text("%x", g);
								ImGui::SameLine();
								text_disabled("ammo pieces)");
								ImGui::SameLine();
							}
						}
					}
					else if (b == button_type::REPLENISHABLE) {
						const auto num_charges = typed_flavour.template get<components::item>().charges;

						if (num_charges > 1) {
							ImGui::SameLine();
							text("(x%x)", num_charges);
							ImGui::SameLine();
						}
					}
				});
			},
			[&](const auto num_affordable) {
				cosm.on_flavour(f_id, [&](const auto& typed_flavour) {
					auto& requested = b == button_type::REPLENISHABLE ? requested_replenishables : requested_weapons;

					if (requested.size() > 0) {
						if (requested.front() == index) {
							requested.erase(requested.begin());

							if (num_affordable > 0) {
								set_result(f_id);
							}
						}
					}

					(void)typed_flavour;

					const auto prev_y = ImGui::GetCursorPosY();

					if (b == button_type::REPLENISHABLE) {
						text_disabled("(Can buy");
						ImGui::SameLine();
						ImGui::SetCursorPosY(prev_y);
						text_color(typesafe_sprintf("%x", num_affordable), num_affordable == 0 ? red : money_color);
						ImGui::SameLine();
						ImGui::SetCursorPosY(prev_y);
						text_disabled("more)");
						ImGui::SameLine();
						ImGui::SetCursorPosY(prev_y);
					}

					const auto space_rhs = num_owned + num_carryable;

					text_disabled("Space:");
					ImGui::SameLine();
					ImGui::SetCursorPosY(prev_y);

					if (num_carryable == 0) {
						text_color(typesafe_sprintf("%x/%x", num_owned, space_rhs), red);
					}
					else {
						text_disabled(typesafe_sprintf("%x/%x", num_owned, space_rhs));
					}
				});
			}
		);
	};

	auto price_comparator = [&](const auto& a, const auto& b) {
		const auto pa = price_of(a);
		const auto pb = price_of(b);

		return pa > pb;
	};

	auto owned_comparator = [&](const auto& a, const auto& b) {
		return price_comparator(a.ammo_piece, b.ammo_piece);
	};

	struct owned_entry {
		item_flavour_id weapon;
		item_flavour_id ammo_piece;
		int instances_owned = 1;

		owned_entry() = default;
		owned_entry(const item_flavour_id& weapon, const item_flavour_id& ammo_piece) : weapon(weapon), ammo_piece(ammo_piece) {};

		bool operator==(const owned_entry& b) const {
			return std::tie(weapon, ammo_piece) == std::tie(b.weapon, b.ammo_piece);
		}
	};

	std::vector<owned_entry> owned_weapons;

	if (subject.alive()) {
		money_type equipment_value = 0;

		subject.for_each_contained_item_recursive([&](const auto& typed_item) {
			if (const auto price = typed_item.find_price(); price.has_value() && *price != 0) {
				/* Skip bombs, personal deposits and zero-price items which are considered non-buyable */
				if (typed_item.get_current_slot().get_type() == slot_function::PERSONAL_DEPOSIT) {
					return;
				}

				if (const auto fuse = typed_item.template find<invariants::hand_fuse>()) {
					if (fuse->is_like_plantable_bomb()) {
						return;
					}
				}

				equipment_value += *price;

				const auto ammo_piece_flavour = ::calc_purchasable_ammo_piece(typed_item);

				if (logically_set(ammo_piece_flavour)) {
					const auto new_entry = owned_entry(typed_item.get_flavour_id(), ammo_piece_flavour);

					if (const auto it = find_in(owned_weapons, new_entry); it != owned_weapons.end()) {
						++it->instances_owned;
					}
					else {
						owned_weapons.push_back(new_entry);
					}
				}
			}
		});

		sort_range(owned_weapons, owned_comparator);
		text("Equipment value:");
		ImGui::SameLine();
		text_color(typesafe_sprintf("%x$", equipment_value), money_color);

		text("Owned weapons:");

		for (const auto& o : owned_weapons) {
			ImGui::SameLine();
			draw_owned_weapon(o.weapon); 

			if (o.instances_owned > 1) {
				ImGui::SameLine();
				text_disabled(typesafe_sprintf("(%xx)", o.instances_owned));
			}
		}

		ImGui::Separator();
	}

	if (in.is_in_buy_zone) {
		if (owned_weapons.size() > 0) {
			std::unordered_set<item_flavour_id> displayed_replenishables;

			centered_text("REPLENISH AMMUNITION");

			if (owned_weapons.size() > 0) {
				ImGui::Separator();
			}

			for (const auto& f : owned_weapons) {
				if (found_in(displayed_replenishables, f.ammo_piece)) {
					continue;
				}

				emplace_element(displayed_replenishables, f.ammo_piece);

				const auto index = 1 + index_in(owned_weapons, f);
				const bool choice_was_made = purchase_item_button(f.ammo_piece, button_type::REPLENISHABLE, index);

				ImGui::Separator();

				if (choice_was_made) {
					const bool maybe_close = false;
					set_result(f.ammo_piece, maybe_close);
				}
			}
		}

#if BUY_CATEGORIES_SEPARATE
		ImGui::BeginChild("Child1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, 0));
#endif

		centered_text("BUY WEAPONS");

		auto category_button = [&](const auto& hotkey_text, const auto ss, const bool active = false) {
			auto darkened = darken_selectables();

			const auto category_button_size = ImVec2(0, standard_button_h * 0.8f);

			const auto before_pos = ImGui::GetCursorPos();

			const auto id = typesafe_sprintf("##%x", ss);
			const bool result = ImGui::Selectable(id.c_str(), active, ImGuiSelectableFlags_None, category_button_size);
			const auto after_pos = ImGui::GetCursorPos();

			ImGui::SetCursorPos(before_pos);
			text_disabled(hotkey_text);
			ImGui::SameLine();
			text(ss);
			ImGui::SetCursorPos(after_pos);

			return result;
		};

		if (current_menu == buy_menu_type::MAIN) {
			centered_text("CHOOSE CATEGORY");

			auto do_buttons_for_buy_categories = [&](const buy_menu_type e) {
				if (e == buy_menu_type::MAIN) {
					return;
				}

				static const auto m = make_hotkey_map();

				const auto bound_key = key_or_default(m, e);

				ensure(bound_key != augs::event::keys::key::INVALID);

				const auto hotkey_text = typesafe_sprintf("(%x)", key_to_string(bound_key));
				const auto label = format_enum(e);

				if (category_button(hotkey_text, label.c_str(), e == current_menu)) {
					next_requested_menu = e;
				}
			};

			augs::for_each_enum_except_bounds(do_buttons_for_buy_categories);

			const auto hotkey = "(" + key_to_string(key_opened) + ")";

			if (category_button(hotkey.c_str(), "Cancel")) {
				hide();
				return result;
			}
		}

#if BUY_CATEGORIES_SEPARATE
		ImGui::EndChild();
#endif

		if (current_menu != buy_menu_type::MAIN) {
#if BUY_CATEGORIES_SEPARATE
			ImGui::SameLine();
			ImGui::BeginChild("Child2", ImVec2(0, 0), false);
#endif
			centered_text(to_uppercase(format_enum(current_menu)));

			auto do_item_menu = [&](
				const std::optional<item_holding_stance> considered_stance,
				auto&& for_each_purchasable
			) {
				std::vector<item_flavour_id> buyable_items;

				for_each_purchasable([&](const auto& id, const auto& flavour) {
					const auto item = flavour.template get<invariants::item>();

					if (!factions_compatible(subject, id)) {
						return;
					}

					if (considered_stance.has_value()) {
						if (item.holding_stance != *considered_stance) {
							const bool but_its_sniper_and_we_want_rifles = item.holding_stance == item_holding_stance::SNIPER_LIKE && considered_stance == item_holding_stance::RIFLE_LIKE;

							if (!but_its_sniper_and_we_want_rifles) {
								return;
							}
						}
					}

					buyable_items.push_back(id);
				});

				sort_range(buyable_items, price_comparator);
				reverse_range(buyable_items);

				for (const auto& b : buyable_items) {
					const auto index = 1 + index_in(buyable_items, b);

					if (price_of(b) > 0) {
						ImGui::Separator();

						if (purchase_item_button(b, button_type::BUYABLE_WEAPON, index)) {
							set_result(b);
						}
					}
				}

				if (!buyable_items.empty()) {
					ImGui::Separator();
				}

				if (category_button("(ESC)", "Cancel")) {
					next_requested_menu = buy_menu_type::MAIN;
				}
			};

			auto do_spells_menu = [&]() {
				std::vector<spell_id> buyable_spells;

				for_each_through_std_get(
					spells,
					[&]<typename S>(const S&) {
						if (!factions_compatible(subject, spell_id::of<S>())) {
							return;
						}

						spell_id id;
						id.set<S>();
						buyable_spells.push_back(id);
					}
				);

				sort_range(buyable_spells, price_comparator);
				reverse_range(buyable_spells);

				for (const auto& b : buyable_spells) {
					const auto index = 1 + index_in(buyable_spells, b);

					if (price_of(b) > 0) {
						ImGui::Separator();

						if (purchase_spell_button(b, index)) {
							set_result(b);
						}
					}
				}

				if (!buyable_spells.empty()) {
					ImGui::Separator();
				}

				if (category_button("(ESC)", "Cancel")) {
					next_requested_menu = buy_menu_type::MAIN;
				}
			};

			auto for_each_gun = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::gun>(callback);
			};

			auto for_each_rifle = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
					if (!is_shotgun_like(cosm, id)) {
						callback(id, flavour);
					}
				});
			};

			auto for_each_melee = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::melee>([&](const auto& id, const auto& flavour) {
					callback(id, flavour);
				});
			};

			auto for_each_pistol = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
					if (price_of(item_flavour_id(id)) <= 1500) {
						callback(id, flavour);
					}
				});
			};

			auto for_each_smg = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
					if (price_of(item_flavour_id(id)) > 1500) {
						callback(id, flavour);
					}
				});
			};

			auto for_each_shotgun = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
					if (is_shotgun_like(cosm, id)) {
						callback(id, flavour);
					}
				});
			};

			auto for_each_tool = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::tool>(
					[&](const auto& id, const auto& flavour) {
						if (::is_backpack_like(flavour)) {
							callback(id, flavour);
						}

						if (!::is_armor_like(flavour)) {
							callback(id, flavour);
						}
					}
				);
			};

			auto for_each_armor = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::tool>(
					[&](const auto& id, const auto& flavour) {
						if (::is_armor_like(flavour)) {
							callback(id, flavour);
						}
					}
				);
			};

			auto for_each_grenade = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::hand_fuse>(callback);
			};

			switch (current_menu) {
				case buy_menu_type::MELEE: {
					do_item_menu(
						item_holding_stance::KNIFE_LIKE,
						for_each_melee
					);
					break;
				}

				case buy_menu_type::PISTOLS: {
					do_item_menu(
						item_holding_stance::PISTOL_LIKE,
						for_each_pistol
					);
					break;
				}

				case buy_menu_type::SUBMACHINE_GUNS: {
					do_item_menu(
						item_holding_stance::PISTOL_LIKE,
						for_each_smg
					);
					break;
				}

				case buy_menu_type::RIFLES: {
					do_item_menu(
						item_holding_stance::RIFLE_LIKE,
						for_each_rifle
					);
					break;
				}

				case buy_menu_type::HEAVY_GUNS: {
					do_item_menu(
						item_holding_stance::HEAVY_LIKE,
						for_each_gun
					);
					break;
				}

				case buy_menu_type::SHOTGUNS: {
					do_item_menu(
						item_holding_stance::RIFLE_LIKE,
						for_each_shotgun
					);
					break;
				}

				case buy_menu_type::SPELLS: {
					do_spells_menu();
					break;
				}

				case buy_menu_type::GRENADES: {
					do_item_menu(
						std::nullopt,
						for_each_grenade
					);

					break;
				}

				case buy_menu_type::ARMORS: {
					do_item_menu(
						std::nullopt,
						for_each_armor
					);

					break;
				}

				case buy_menu_type::TOOLS: {
					do_item_menu(
						std::nullopt,
						for_each_tool
					);

					break;
				}

				default: break;
			}

#if BUY_CATEGORIES_SEPARATE
			ImGui::EndChild();
#endif
		}
		else {
			requested_weapons.clear();
		}
	}
	else {
		text_color("You are not in the designated buy area!", red);
	}

	current_menu = next_requested_menu;
	return result;
}
