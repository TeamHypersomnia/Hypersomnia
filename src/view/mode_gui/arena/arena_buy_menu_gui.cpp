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
#include "game/detail/flavour_scripts.h"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/organization/special_flavour_id_types.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "view/mode_gui/arena/arena_mode_gui_settings.h"

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

	/* if (!show) { */
	/* 	return std::nullopt; */
	/* } */

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

	auto image_of = [&](const auto& of) {
		if constexpr(std::is_same_v<decltype(of), const item_flavour_id&>) {
			return cosm.on_flavour(of, [&](const auto& typed_flavour) {
				return typed_flavour.get_image_id();
			});
		}
		else {
			return cosm[of].get_image_id();
		}
	};

	auto price_of = [&](const auto& of) {
		if constexpr(std::is_same_v<decltype(of), const item_flavour_id&>) {
			return cosm.on_flavour(of, [&](const auto& typed_flavour) {
				return *typed_flavour.find_price();
			});
		}
		else {
			return *cosm[of].find_price();
		}
	};

	enum class button_type {
		REPLENISHABLE,
		OWNED_WEAPON,
		BUYABLE_WEAPON
	};

	auto flavour_button = [&](const item_flavour_id& f_id, const button_type b, const std::string& label = "") {
		if (const auto& entry = in.images_in_atlas.at(image_of(f_id)).diffuse; entry.exists()) {
			const auto local_pos = ImGui::GetCursorPos();

			const auto& item_spacing = ImGui::GetStyle().ItemSpacing;
			//auto scp3 = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(3, 10));

			const auto size = vec2(entry.get_original_size());

			const auto line_h = ImGui::GetTextLineHeight();
			const bool show_description = b != button_type::OWNED_WEAPON;

			const auto num_owned = subject.count_contained(f_id);

			const auto slot_opts = slot_finding_opts {
				slot_finding_opt::CHECK_WEARABLES,
				slot_finding_opt::CHECK_HANDS,
				slot_finding_opt::CHECK_CONTAINERS
			};

			const auto num_carryable = num_carryable_pieces(
				subject,
				slot_opts, 
				f_id
			);

			const auto price = price_of(f_id);

			const auto num_affordable = in.available_money / price;

			const bool is_disabled = num_affordable == 0 || num_carryable == 0;

			if (show_description) {
				const auto hover_col = rgba(ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
				const auto active_col = rgba(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);

				auto scp = scoped_style_color(ImGuiCol_HeaderHovered, rgba(hover_col).multiply_rgb(1 / 2.1f));
				auto scp2 = scoped_style_color(ImGuiCol_HeaderActive, rgba(active_col).multiply_rgb(1 / 1.8f));

				const auto label_id = typesafe_sprintf("##%xlabel");


				if (is_disabled) {
					const auto selectable_size = ImVec2(ImGui::GetContentRegionAvailWidth(), 2 * item_spacing.y - 1 + std::max(size.y, line_h * 2));
					rect_filled(selectable_size, in.settings.disabled_bg, vec2(0, -item_spacing.y + 2));
				}
				else {
					const auto selectable_size = ImVec2(0, item_spacing.y - 1 + std::max(size.y, line_h * 2));
					ImGui::Selectable(label_id.c_str(), false, ImGuiSelectableFlags_None, selectable_size);
				}

				ImGui::SetCursorPos(local_pos);

				text_disabled(typesafe_sprintf("(%x)", label));
				ImGui::SameLine();
			}

			const auto image_padding = vec2(0, 4);
			game_image(entry, size, white, image_padding);
			invisible_button(label, size + image_padding);

			if (show_description) {
				ImGui::SameLine();

				cosm.on_flavour(f_id, [&](const auto& typed_flavour) {
					const auto x = ImGui::GetCursorPosX();
					text(typed_flavour.get_name());
					ImGui::SameLine();

					const auto prev_y = ImGui::GetCursorPosY() + line_h;
					ImGui::SetCursorPosY(prev_y);
					ImGui::SetCursorPosX(x);

					text_color(typesafe_sprintf("%x$", price_of(f_id)), money_color);


					ImGui::SameLine();

					//if (num_affordable > 0) {
						text_disabled("(Can buy");
						ImGui::SameLine();
						text_color(typesafe_sprintf("%x", num_affordable), num_affordable == 0 ? red : money_color);
						ImGui::SameLine();
						text_disabled("more)");
						ImGui::SameLine();
						//}
					//else {
						//text_color("Insufficient funds", red);
						//ImGui::SameLine();
						//}

					const auto space_rhs = num_owned + num_carryable;

					text_disabled("Space:");
					ImGui::SameLine();
					if (num_carryable == 0) {
						text_color(typesafe_sprintf("%x/%x", num_owned, space_rhs), red);
					}
					else {
						text_disabled(typesafe_sprintf("%x/%x", num_owned, space_rhs));
					}

					ImGui::SetCursorPosY(prev_y + line_h + item_spacing.y);
				});
			}

			return false;
		}

		return false;
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
			flavour_button(o.weapon, button_type::OWNED_WEAPON, "");

			if (o.num > 1) {
				ImGui::SameLine();
				text_disabled(typesafe_sprintf("(%xx)", o.num));
			}
		}

		ImGui::Separator();
	}

	result_type result;

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

				const auto index = typesafe_sprintf("S+%x", 1 + index_in(owned_weapons, f));
				const bool choice_was_made = flavour_button(f.mag, button_type::REPLENISHABLE, index);

				ImGui::Separator();

				if (choice_was_made) {
					mode_commands::item_purchase msg;
					msg.flavour_id = f.mag;
					result = msg;
				}
			}
		}

		ImGui::BeginChild("Child1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, 0));

		centered_text("BUY WEAPONS");

		augs::for_each_enum_except_bounds([&](const buy_menu_type e) {
			if (e == buy_menu_type::MAIN) {
				return;
			}

			const auto index = typesafe_sprintf("(%x)", static_cast<int>(e));

			text_disabled(index);
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
				auto&& for_each_potential_item
			) {
				std::vector<item_flavour_id> buyable_items;

				for_each_potential_item([&](const auto& id, const auto& flavour) {
					if (considered_stance != std::nullopt) {
						const auto item = flavour.template find<invariants::item>();

						if (item->holding_stance != *considered_stance) {
							return;
						}
					}

					buyable_items.push_back(id);
				});

				sort_range(buyable_items, price_comparator);
				reverse_range(buyable_items);

				for (const auto& b : buyable_items) {
					const auto index = typesafe_sprintf("%x", 1 + index_in(buyable_items, b));

					ImGui::Separator();

					if (flavour_button(b, button_type::BUYABLE_WEAPON, index)) {
						mode_commands::item_purchase msg;
						msg.flavour_id = b;
						result = msg;
					}
				}

				if (!buyable_items.empty()) {
					ImGui::Separator();
				}
			};

			auto for_each_gun = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::gun>(callback);
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
					const auto& gun_def = flavour.template get<invariants::gun>();

					if (gun_def.shot_cooldown_ms > 150) {
						/* A heavy gun with such a slow firerate must be a shotgun */
						callback(id, flavour);
					}
				});
			};

			switch (current_menu) {
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
						for_each_gun
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
						item_holding_stance::HEAVY_LIKE,
						for_each_shotgun
					);
					break;
				}

				default: break;
			}

			ImGui::EndChild();
		}
	}
	else {
		text_color("You are not in the designated buy area!", red);
	}

	return result;
}
