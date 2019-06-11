#pragma once
#include "view/game_gui/special_indicator.h"

template <class T, class MI>
void gather_special_indicators(
	const T& mode, 
	const MI& mode_input, 
	const faction_type viewer_faction,
	const necessary_images_in_atlas_map& necessarys,
	std::vector<special_indicator>& special_indicators,
	special_indicator_meta& meta
) {
	if constexpr(std::is_same_v<T, bomb_mode>) {
		mode.on_bomb_entity(
			mode_input,
			[&](const auto& bomb) {
				if constexpr(!is_nullopt_v<decltype(bomb)>) {
					const auto participants = mode.calc_participating_factions(mode_input);
					const auto capability = bomb.get_owning_transfer_capability();
					meta.bomb_owner = capability;

					if (viewer_faction == participants.bombing) {
						if (capability.dead() || capability.get_official_faction() != viewer_faction) {
							special_indicators.push_back({
								bomb.get_logic_transform(),
								white,

								necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),
								necessarys.at(assets::necessary_image_id::BOMB_INDICATOR)
							});
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
