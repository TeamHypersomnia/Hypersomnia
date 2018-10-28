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
#include "game/detail/buy_area_in_range.h"

static auto make_hotkey_map() {
	using namespace augs::event::keys;

	arena_buy_menu_hotkey_map m;
	m[key::Q] = buy_menu_type::MELEE;
	m[key::W] = buy_menu_type::PISTOLS;
	m[key::E] = buy_menu_type::SUBMACHINE_GUNS;
	m[key::R] = buy_menu_type::RIFLES;
	m[key::T] = buy_menu_type::SHOTGUNS;
	m[key::Y] = buy_menu_type::HEAVY_GUNS;
	m[key::U] = buy_menu_type::GRENADES;
	m[key::I] = buy_menu_type::SPELLS;
	m[key::O] = buy_menu_type::TOOLS;

	return m;
}

bool arena_buy_menu_gui::control(app_ingame_intent_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (const auto it = mapped_or_nullptr(in.controls, key)) {
			if (*it == app_ingame_intent_type::OPEN_BUY_MENU) {
				show = !show;
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

		if (const auto it = mapped_or_nullptr(m, key)) {
			current_menu = *it;
			return true;
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
using result_type = std::optional<mode_commands::item_purchase>;

result_type arena_buy_menu_gui::perform_imgui(const input_type in) {
	using namespace augs::imgui;
	(void)in;

	const auto& subject = in.subject;
	const auto& cosm = subject.get_cosmos();
	const auto money_color = in.money_indicator_color;

	if (subject.dead()) {
		return std::nullopt;
	}

	if (!buy_area_in_range(subject)) {
		show = false;
		return std::nullopt;
	}

	if (!show) {
		return std::nullopt;
	}

	ImGui::SetNextWindowPosCenter();

	ImGui::SetNextWindowSize((vec2(ImGui::GetIO().DisplaySize) * 0.5f).operator ImVec2(), ImGuiCond_FirstUseEver);

	const auto window_name = "Buy menu";
	auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar);

	centered_text(window_name);

	if (const auto& entry = in.images_in_atlas.at(in.money_icon).diffuse; entry.exists()) {
		game_image_button("#moneyicon", entry);
		ImGui::SameLine();
	}

	text_color(typesafe_sprintf("%x$", in.available_money), money_color);

	const auto& spells = cosm.get_common_significant().spells;

	auto _on_spell = [&](const spell_id& id, auto&& f) -> decltype(auto) {
		return ::on_spell(cosm, id, std::forward<decltype(f)>(f));
	};

	auto image_of = [&](const auto& of) {
		if constexpr(std::is_same_v<decltype(of), const item_flavour_id&>) {
			if (!is_alive(cosm, of)) {
				return assets::image_id();
			}

			return cosm.on_flavour(of, [&](const auto& typed_flavour) {
				return typed_flavour.get_image_id();
			});
		}
		else {
			return _on_spell(of, [&](const auto& spell_data) {
				return spell_data.appearance.icon;
			});
		}
	};

	auto price_of = [&](const auto& of) {
		return *::find_price_of(cosm, of);
	};

	enum class button_type {
		REPLENISHABLE,
		BUYABLE_WEAPON
	};

	auto draw_owned_weapon = [&](const item_flavour_id& f_id) {
		if (const auto& entry = in.images_in_atlas.at(image_of(f_id)).diffuse; entry.exists()) {
			const auto image_padding = vec2(0, 4);
			const auto size = vec2(entry.get_original_size());
			game_image(entry, size, white, image_padding);
			invisible_button("", size + image_padding);
		}
	};

	result_type result;

	auto set_result = [&](const auto& r) {
		mode_commands::item_purchase msg;

		if constexpr(std::is_same_v<decltype(r), const item_flavour_id&>) {
			msg.item = r;
		}
		else {
			msg.spell = r;
		}

		result = msg;
	};

	auto general_purchase_button = [&](
		const auto& object, 
		const auto& selected,
		const bool is_owned,
		const std::optional<int> num_carryable,
		const int index,
		const std::string& additional_id,
		const auto& hotkey_text,
		auto&& name_callback,
		auto&& price_callback
	) {
		const auto& entry = in.images_in_atlas.at(image_of(object)).diffuse;

		if (!entry.exists()) {
			return false;
		}

		const auto price = price_of(object);

		if (price == 0) {
			return false;
		}

		const auto local_pos = ImGui::GetCursorPos();

		const auto& item_spacing = ImGui::GetStyle().ItemSpacing;
		const auto size = vec2(entry.get_original_size());

		const auto line_h = ImGui::GetTextLineHeight();
		const auto button_h = std::max(size.y, line_h * 2) + item_spacing.y;

		const auto num_affordable = in.available_money / price;

		const bool is_disabled = num_affordable == 0 || (num_carryable && *num_carryable == 0);

		bool result = false;

		const auto hover_col = rgba(ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
		const auto active_col = rgba(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);

		auto scp = scoped_style_color(ImGuiCol_HeaderHovered, rgba(hover_col).multiply_rgb(1 / 2.1f));
		auto scp2 = scoped_style_color(ImGuiCol_HeaderActive, rgba(active_col).multiply_rgb(1 / 1.8f));

		const auto label_id = typesafe_sprintf("##%xr%x", index, additional_id);

		const bool active = found_in(selected, index);

		if (is_owned) {
			const auto col = active ? in.settings.already_owns_active_bg : in.settings.already_owns_bg;
			const auto selectable_size = ImVec2(ImGui::GetContentRegionAvailWidth(), 2 * item_spacing.y + button_h);
			rect_filled(selectable_size, col, vec2(0, -item_spacing.y + 1));
		}
		else if (is_disabled) {
			const auto col = active ? in.settings.disabled_active_bg : in.settings.disabled_bg;
			const auto selectable_size = ImVec2(ImGui::GetContentRegionAvailWidth(), 2 * item_spacing.y + button_h);
			rect_filled(selectable_size, col, vec2(0, -item_spacing.y + 1));
		}
		else {
			const auto selectable_size = ImVec2(0, 1 + button_h);
			result = ImGui::Selectable(label_id.c_str(), active, ImGuiSelectableFlags_None, selectable_size);
		}

		ImGui::SetCursorPos(local_pos);

		text_disabled(hotkey_text);
		ImGui::SameLine();

		const auto image_padding = vec2(0, 4);
		game_image(entry, size, white, image_padding);
		invisible_button("", size + image_padding);

		ImGui::SameLine();

		const auto x = ImGui::GetCursorPosX();
		name_callback();

		const auto prev_y = ImGui::GetCursorPosY() + line_h;
		ImGui::SetCursorPosY(prev_y);
		ImGui::SetCursorPosX(x);

		if (is_owned) {
			text("Owned already");
		}
		else {
			text_color(typesafe_sprintf("%x$", price_of(object)), money_color);
			ImGui::SameLine();

			price_callback(num_affordable);
		}

		ImGui::SetCursorPosY(local_pos.y + button_h + item_spacing.y);

		return result;
	};

	auto purchase_spell_button = [&](
		const spell_id& s_id, 
		const int index
	) {
		auto& requested = requested_weapons;
		const auto& selected = selected_weapons;

		if (requested.size() > 0) {
			if (requested.front() == index) {
				requested.erase(requested.begin());
				set_result(s_id);
			}
		}

		const auto additional_id = "s";
		const auto hotkey_text = typesafe_sprintf("(%x)", index);

		static const augs::baked_font dummy; 
		/* In case we make the font statically allocated one day */
		static_assert(sizeof(dummy) < 1000);

		const bool learnt = subject.template get<components::sentience>().is_learnt(s_id);

		return general_purchase_button(
			s_id,
			selected,
			learnt,
			1,
			index,
			additional_id,
			hotkey_text,
			[&]() {
				_on_spell(s_id, [&](const auto& spell_data) {
					const auto f = augs::gui::text::from_bbcode(spell_data.appearance.name, { dummy, white });

					if (f.empty()) {
						return;
					}

					const auto col = f[0].format.color;
					const auto ss = f.operator std::string();

					text_color(ss, col);
					ImGui::SameLine();
				});
			},
			[&](auto num_affordable) {
				num_affordable = std::min(num_affordable, 1);

				_on_spell(s_id,  [&](const auto& spell_data) {
					(void)spell_data;
					text_disabled("(Can buy");
					ImGui::SameLine();
					text_color(typesafe_sprintf("%x", num_affordable), num_affordable == 0 ? red : money_color);
					ImGui::SameLine();
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

		auto& requested = b == button_type::REPLENISHABLE ? requested_replenishables : requested_weapons;
		const auto& selected = b == button_type::REPLENISHABLE ? selected_replenishables : selected_weapons;

		if (requested.size() > 0) {
			if (requested.front() == index) {
				requested.erase(requested.begin());
				set_result(f_id);
			}
		}

		const auto num_carryable = num_carryable_pieces(
			subject,
			::get_buy_slot_opts(), 
			f_id
		);

		const auto additional_id = std::to_string(int(b));

		const bool is_replenishable = b == button_type::REPLENISHABLE;
		const auto hotkey_text = typesafe_sprintf(is_replenishable ? "(Shift+%x)" : "(%x)", index);

		const auto num_owned = subject.count_contained(f_id);

		const bool owned_status = [&]() {
			if (::makes_sense_to_only_own_one(cosm, f_id)) {
				return num_owned >= 1;
			}

			return false;
		}();

		return general_purchase_button(
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
								text_disabled("mags)");
								ImGui::SameLine();
							}
						}
					}
				});
			},
			[&](const auto num_affordable) {
				cosm.on_flavour(f_id, [&](const auto& typed_flavour) {
					(void)typed_flavour;
					text_disabled("(Can buy");
					ImGui::SameLine();
					text_color(typesafe_sprintf("%x", num_affordable), num_affordable == 0 ? red : money_color);
					ImGui::SameLine();
					text_disabled("more)");
					ImGui::SameLine();

					const auto space_rhs = num_owned + num_carryable;

					text_disabled("Space:");
					ImGui::SameLine();

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

		if (pa == pb) {
			return image_of(a).indirection_index < image_of(b).indirection_index;
		}

		return pa > pb;
	};

	auto owned_comparator = [&](const auto& a, const auto& b) {
		return price_comparator(a.mag, b.mag);
	};

	struct owned_entry {
		item_flavour_id weapon;
		item_flavour_id mag;
		int num = 1;

		owned_entry() = default;
		owned_entry(const item_flavour_id& weapon, const item_flavour_id& mag) : weapon(weapon), mag(mag) {};

		bool operator==(const owned_entry& b) const {
			return std::tie(weapon, mag) == std::tie(b.weapon, b.mag);
		}
	};

	std::vector<owned_entry> owned_weapons;

	if (subject.alive()) {
		money_type equipment_value = 0;

		subject.for_each_contained_item_recursive([&](const auto& typed_item) {
			if (const auto price = typed_item.find_price(); price != std::nullopt && *price != 0) {
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

				if (const auto mag = typed_item[slot_function::GUN_DETACHABLE_MAGAZINE]) {
					const auto new_entry = owned_entry(typed_item.get_flavour_id(), mag->only_allow_flavour);

					if (const auto it = find_in(owned_weapons, new_entry); it != owned_weapons.end()) {
						++it->num;
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

			if (o.num > 1) {
				ImGui::SameLine();
				text_disabled(typesafe_sprintf("(%xx)", o.num));
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
				if (found_in(displayed_replenishables, f.mag)) {
					continue;
				}

				emplace_element(displayed_replenishables, f.mag);

				const auto index = 1 + index_in(owned_weapons, f);
				const bool choice_was_made = purchase_item_button(f.mag, button_type::REPLENISHABLE, index);

				ImGui::Separator();

				if (choice_was_made) {
					set_result(f.mag);
				}
			}
		}

		ImGui::BeginChild("Child1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, 0));

		centered_text("BUY WEAPONS");

		augs::for_each_enum_except_bounds([&](const buy_menu_type e) {
			if (e == buy_menu_type::MAIN) {
				return;
			}

			static const auto m = make_hotkey_map();

			const auto bound_key = key_or_default(m, e);

			ensure(bound_key != augs::event::keys::key::INVALID);

			const auto hotkey_text = typesafe_sprintf("(%x)", key_to_string(bound_key));

			text_disabled(hotkey_text);
			ImGui::SameLine();

			const auto label = format_enum(e);

			if (ImGui::Selectable(label.c_str(), e == current_menu)) {
				current_menu = e;
			}
		});

		ImGui::EndChild();

		if (current_menu != buy_menu_type::MAIN) {
			ImGui::SameLine();
			ImGui::BeginChild("Child2", ImVec2(0, 0), false);
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

					if (considered_stance != std::nullopt) {
						if (item.holding_stance != *considered_stance) {
							return;
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
			};

			auto do_spells_menu = [&]() {
				std::vector<spell_id> buyable_spells;

				for_each_through_std_get(
					spells,
					[&](const auto& typed_spell) {
						using S = remove_cref<decltype(typed_spell)>;

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
					if (price_of(item_flavour_id(id)) <= 1000) {
						callback(id, flavour);
					}
				});
			};

			auto for_each_smg = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
					if (price_of(item_flavour_id(id)) > 1000) {
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
				cosm.for_each_flavour_having<invariants::tool>(callback);

				cosm.for_each_flavour_having<invariants::container, invariants::item>(
					[&](const auto& id, const auto& flavour) {
						if (::is_backpack_like(flavour)) {
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

				case buy_menu_type::TOOLS: {
					do_item_menu(
						std::nullopt,
						for_each_tool
					);

					break;
				}

				default: break;
			}

			ImGui::EndChild();
		}
		else {
			requested_weapons.clear();
		}
	}
	else {
		text_color("You are not in the designated buy area!", red);
	}

	return result;
}
