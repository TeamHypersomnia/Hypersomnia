#pragma once
#include "game/modes/test_mode.h"
#include "view/game_gui/special_indicator.h"
#include "game/detail/hand_fuse_math.h"
#include "game/modes/arena_mode.hpp"

template <class T, class MI, class E>
void gather_special_indicators(
	const T& mode, 
	const MI& mode_input, 
	const faction_type viewer_faction,
	const necessary_images_in_atlas_map& necessarys,
	std::vector<special_indicator>& special_indicators,
	special_indicator_meta& meta,
	const E& viewed_character
) {
	if constexpr(std::is_same_v<T, arena_mode>) {
		meta.draw_nicknames_for_fallback = mode.get_current_fallback_color_for(mode_input, viewer_faction);

		mode.on_bomb_entity(
			mode_input,
			[&](const auto& bomb) {
				if constexpr(!is_nullopt_v<decltype(bomb)>) {
					const auto participants = mode.calc_participating_factions(mode_input);
					const auto capability = bomb.get_owning_transfer_capability();
					meta.bomb_owner = capability;

					if (viewer_faction == participants.bombing) {
						if (capability.dead() || capability.get_official_faction() != viewer_faction) {
							if (const auto fuse = bomb.template find<components::hand_fuse>()) {
								/* Draw the bomb icon on screen only if it is unarmed yet. */
								const bool armed = fuse->armed();
								const bool draw_onscreen = !armed;

								auto col = white;

								if (armed) {
									const auto fuse_math = beep_math { *fuse, bomb.template get<invariants::hand_fuse>(), viewed_character.get_cosmos().get_clock() };
									const auto mult = fuse_math.get_beep_light_mult();

									col.multiply_rgb(mult);
									col.r = 255;
								}

								special_indicators.push_back({
									bomb.get_logic_transform(),
									col,

									necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),
									necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),

									draw_onscreen
								});
							}
						}
					}
					else if (viewer_faction == participants.defusing) {
						if (const auto fuse = bomb.template find<components::hand_fuse>()) {
							if (fuse->armed()) {
								meta.now_defusing = fuse->character_now_defusing;
							}
						}
					}
				}
			}
		);
	}
	else if constexpr(std::is_same_v<T, test_mode>) {

	}
	else {
		static_assert(always_false_v<T>, "Unhandled mode type!");
	}
}
