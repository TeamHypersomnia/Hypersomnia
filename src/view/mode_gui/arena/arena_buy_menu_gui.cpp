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

	text(typesafe_sprintf("%x$"), in.available_money);

	auto image_of = [&](const auto& of) {
		if constexpr(std::is_same_v<decltype(of), const entity_flavour_id&>) {
			return cosm.on_flavour(of, [&](const auto& typed_flavour) {
				return typed_flavour.get_image_id();
			});
		}
		else {
			return cosm[of].get_image_id();
		}
	};

	auto price_of = [&](const auto& of) {
		if constexpr(std::is_same_v<decltype(of), const entity_flavour_id&>) {
			return cosm.on_flavour(of, [&](const auto& typed_flavour) {
				return typed_flavour.find_price();
			});
		}
		else {
			return *cosm[of].find_price();
		}
	};

	auto flavour_button = [&](const entity_flavour_id& f_id, const std::string& label = "") {
		if (const auto& entry = in.images_in_atlas.at(image_of(f_id)).diffuse; entry.exists()) {
			const bool show_description = !label.empty();

			if (show_description) {
				text_disabled(typesafe_sprintf("(%x)", label));
				ImGui::SameLine();
			}

			const auto result = game_image_button(label, entry);

			if (show_description) {
				ImGui::SameLine();

				cosm.on_flavour(f_id, [&](const auto& typed_flavour) {
					const auto x = ImGui::GetCursorPosX();
					text(typed_flavour.get_name());
					ImGui::SameLine();

					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeight());
					ImGui::SetCursorPosX(x);

					text_color(typesafe_sprintf("%x$", price_of(f_id)), in.money_indicator_color);
				});
			}

			return result;
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
		entity_flavour_id weapon;
		entity_flavour_id mag;
		int num = 1;

		owned_entry() = default;
		owned_entry(const entity_flavour_id& weapon, const entity_flavour_id& mag) : weapon(weapon), mag(mag) {};

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
		text(typesafe_sprintf("Equipment value: %x$", equipment_value));

		text("Owned weapons:");

		for (const auto& o : owned_weapons) {
			ImGui::SameLine();
			flavour_button(o.weapon);

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
			centered_text("REPLENISH AMMUNITION");

			for (const auto& f : owned_weapons) {
				const auto index = typesafe_sprintf("S+%x", 1 + index_in(owned_weapons, f));
				const bool choice_was_made = flavour_button(f.mag, index);

				ImGui::Separator();

				if (choice_was_made) {
					mode_commands::item_purchase msg;
					msg.flavour_id = f.mag;
					result = msg;
				}
			}
		}

		if (current_menu == buy_menu_type::MAIN) {
			centered_text("CHOOSE CATEGORY");

			augs::for_each_enum_except_bounds([&](const buy_menu_type e) {
				if (e == buy_menu_type::MAIN) {
					return;
				}

				const auto index = typesafe_sprintf("(%x)", static_cast<int>(e));

				text_disabled(index);
				ImGui::SameLine();

				const auto label = format_enum(e);

				if (ImGui::Button(label.c_str())) {
					current_menu = e;
				}
			});
		}
		else {
			centered_text(to_uppercase(format_enum(current_menu)));

			if (ImGui::Button("Back")) {
				current_menu = buy_menu_type::MAIN;
			}

			auto do_item_menu = [&](
				const std::optional<item_holding_stance> considered_stance,
				auto&& for_each_potential_item
			) {
				std::vector<entity_flavour_id> buyable_items;

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

					if (flavour_button(b, index)) {
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
					if (price_of(entity_flavour_id(id)) <= 1000) {
						callback(id, flavour);
					}
				});
			};

			auto for_each_smg = [&](auto&& callback) {
				cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
					if (price_of(entity_flavour_id(id)) > 1000) {
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
		}
	}
	else {
		text_color("You are not in the designated buy area!", red);
	}

	return result;
}
