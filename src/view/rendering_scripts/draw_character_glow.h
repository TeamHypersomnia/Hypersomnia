#pragma once

#include "rendering_scripts.h"
#include "augs/drawing/drawing.hpp"
#include "augs/templates/get_by_dynamic_id.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/sprite_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

#include "game/detail/sentience/tool_getters.h"

struct draw_character_glow_in {
	const augs::drawer output;
	const interpolation_system& interpolation;
	const double global_time_seconds;
	const augs::atlas_entry cast_highlight_tex;
};

template <class E>
void draw_character_glow(const E& it, const draw_character_glow_in in) {
	const auto& cosm = it.get_cosmos();
	const auto dt = cosm.get_fixed_delta();

	const auto rigid_body = it.template get<components::rigid_body>();
	const auto teleport_alpha = rigid_body.get_teleport_alpha();

	const auto& sentience = it.template get<components::sentience>();
	const auto casted_spell = sentience.currently_casted_spell;

	if (casted_spell.is_set()) {
		casted_spell.dispatch(
			[&](auto s) {
				const auto highlight_amount = teleport_alpha * (1.f - (in.global_time_seconds - sentience.time_of_last_spell_cast.in_seconds(dt)) / 0.4f);

				if (highlight_amount > 0.f) {
					using S = decltype(s);
					const auto& spell_data = std::get<S>(cosm.get_common_significant().spells);

					auto highlight_col = spell_data.common.associated_color;
					highlight_col.a = static_cast<rgba_channel>(255 * highlight_amount);

					if (const auto tr = it.find_viewing_transform(in.interpolation)) {
						in.output.aabb_centered(
							in.cast_highlight_tex,
							tr->pos,
							highlight_col
						);
					}
				}
			}
		);
	}

	if (const auto absorption = find_active_pe_absorption(it)) {
		const auto shield = cosm[absorption->second];

		if (const auto tool = shield.template find<invariants::tool>()) {
			if (tool->glow_color.a > 0) {
				const auto& personal_electricity = sentience.template get<personal_electricity_meter_instance>();
				const auto highlight_amount = std::sqrt(personal_electricity.get_ratio());

				const auto radius = std::max(10.f, 190.f * highlight_amount);

				if (const auto tr = it.find_viewing_transform(in.interpolation)) {
					auto color = tool->glow_color;

					if (color == rgba::faction_color) {
						if (it.get_official_faction() == faction_type::RESISTANCE) {
							color = rgba(255, 90, 0, 255);
						}

						if (it.get_official_faction() == faction_type::METROPOLIS) {
							color = rgba(0, 146, 222, 255);
						}
					}

					color.mult_alpha(teleport_alpha);

					in.output.aabb_centered(
						in.cast_highlight_tex,
						tr->pos,
						vec2i::square(radius),
						color
					);
				}
			}
		}
	}
}
