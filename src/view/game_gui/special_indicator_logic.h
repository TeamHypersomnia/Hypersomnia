#pragma once
#include <algorithm>
#include "game/modes/test_mode.h"
#include "view/game_gui/special_indicator.h"
#include "game/detail/hand_fuse_math.h"
#include "game/modes/arena_mode.hpp"
#include "view/game_drawing_settings.h"
#include "game/cosmos/for_each_entity.h"
#include "view/audiovisual_state/systems/minimap_sighting_system.h"

template <class T, class MI, class E>
void gather_special_indicators(
	const T& mode,
	const MI& mode_input,
	const faction_type viewer_faction,
	const necessary_images_in_atlas_map& necessarys,
	std::vector<special_indicator>& special_indicators,
	special_indicator_meta& meta,
	const E& viewed_character,
	const game_drawing_settings& drawing,
	const std::vector<minimap_sighting_system::death_record>& recent_deaths
) {
	/*
		Deaths of both enemies and teammates are marked with the skull icon.
		The colors deliberately avoid the assigned per-player colors -
		enemies use the same red as the minimap dots, teammates a friendly color.
		Death events are recorded view-side per logic step, so this works
		even in modes with instant respawns.
	*/

	{
		const auto now = viewed_character.get_cosmos().get_total_seconds_passed();

		const auto show_secs = drawing.show_death_indicator_for_seconds;
		const auto fade_secs = drawing.fade_death_indicator_for_seconds;

		for (const auto& rec : recent_deaths) {
			const auto since = static_cast<float>(now - rec.when);

			if (since < 0.f || since > show_secs) {
				continue;
			}

			auto col = white;

			if (fade_secs > 0.f && since > show_secs - fade_secs) {
				col.mult_alpha(std::clamp((show_secs - since) / fade_secs, 0.f, 1.f));
			}

			special_indicators.push_back({
				transformr(rec.pos),
				col,

				necessarys.at(assets::necessary_image_id::DEATH_INDICATOR),
				necessarys.at(assets::necessary_image_id::DEATH_INDICATOR),

				true
			});
		}
	}

	/*
		Shared by the arena and test modes.
	*/

	auto push_bomb_on_ground_indicator = [&](
		const transformr where,
		const components::hand_fuse& fuse,
		const invariants::hand_fuse& fuse_def
	) {
		const bool defused = fuse.defused();
		const bool armed = fuse.armed();

		/* Draw the bomb icon on screen only if it is unarmed yet. */
		const bool draw_onscreen = !armed;

		auto col = defused ? rgba(160, 160, 160, 255) : white;

		if (armed) {
			const auto fuse_math = beep_math { fuse, fuse_def, viewed_character.get_cosmos().get_clock() };

			col.multiply_rgb(fuse_math.get_beep_light_mult());
			col.r = 255;
		}

		special_indicators.push_back({
			where,
			col,

			necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),
			necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),

			draw_onscreen
		});
	};

	auto push_bomb_carrier_indicator = [&](const auto& capability) {
		const bool carried_by_viewed = entity_id(capability.get_id()) == entity_id(viewed_character.get_id());
		const auto carrier_color = carried_by_viewed ? drawing.minimap.player_color : drawing.minimap.teammate_color;

		/*
			Only on the minimap - the offscreen indicator
			of the carrying teammate already shows the bomb.
		*/
		special_indicators.push_back({
			capability.get_logic_transform(),
			carrier_color,

			necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),
			necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),

			false,
			true
		});
	};

	if constexpr(std::is_same_v<T, arena_mode>) {
		meta.draw_nicknames_for_fallback = mode.get_current_fallback_color_for(mode_input, viewer_faction);

		mode.on_bomb_entity(
			mode_input,
			[&](const auto& bomb) {
				if constexpr(!is_nullopt_v<decltype(bomb)>) {
					const auto participants = mode.calc_participating_factions(mode_input);
					const auto capability = bomb.get_owning_transfer_capability();
					meta.bomb_owner = capability;

					if (const auto fuse = bomb.template find<components::hand_fuse>()) {
						const bool defused = fuse->defused();
						const bool armed = fuse->armed();

						const bool mt_stole_bomb = capability.alive() && capability.get_official_faction() != viewer_faction;
						if (capability.dead() || mt_stole_bomb) {
							/*
								If bomb is armed, draw for MT as well.
								If unarmed, only Resistance sees it.
							*/

							if (viewer_faction == participants.bombing || armed || defused) {
								push_bomb_on_ground_indicator(
									bomb.get_logic_transform(),
									*fuse,
									bomb.template get<invariants::hand_fuse>()
								);
							}
						}
						else {
							const bool teammate_carries_bomb =
								capability.alive() &&
								capability.get_official_faction() == viewer_faction
							;

							if (teammate_carries_bomb) {
								push_bomb_carrier_indicator(capability);
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
		/*
			No bomb bookkeeping in the test mode - scan the cosmos
			for a plantable bomb, so it shows on the minimap
			at the shooting range as well.
		*/

		const auto& cosm = viewed_character.get_cosmos();

		cosm.template for_each_having<components::hand_fuse>(
			[&](const auto& typed_bomb) {
				const auto& fuse_def = typed_bomb.template get<invariants::hand_fuse>();

				if (!fuse_def.is_like_plantable_bomb()) {
					return;
				}

				const auto& fuse = typed_bomb.template get<components::hand_fuse>();
				const auto capability = typed_bomb.get_owning_transfer_capability();

				meta.bomb_owner = capability;

				if (capability.dead()) {
					push_bomb_on_ground_indicator(
						typed_bomb.get_logic_transform(),
						fuse,
						fuse_def
					);
				}
				else {
					push_bomb_carrier_indicator(capability);
				}
			}
		);
	}
	else {
		static_assert(always_false_v<T>, "Unhandled mode type!");
	}
}
