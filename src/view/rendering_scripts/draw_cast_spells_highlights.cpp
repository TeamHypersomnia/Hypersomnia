#include "rendering_scripts.h"
#include "augs/drawing/drawing.h"
#include "augs/templates/get_by_dynamic_id.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/sprite_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void draw_cast_spells_highlights(const draw_cast_spells_highlights_input in) {
	const auto& cosm = in.cosm;
	const auto dt = cosm.get_fixed_delta();

	cosm.for_each_having<components::sentience>(
		[&](const auto it) {
			const auto& sentience = it.template get<components::sentience>();
			const auto casted_spell = sentience.currently_casted_spell;

			if (casted_spell.is_set()) {
				casted_spell.dispatch(
					[&](auto s) {
						const auto highlight_amount = 1.f - (in.global_time_seconds - sentience.time_of_last_spell_cast.in_seconds(dt)) / 0.4f;

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
		}
	);
}

void draw_explosion_body_highlights(const draw_explosion_body_highlights_input in) {
	const auto& cosm = in.cosm;

	cosm.for_each_having<invariants::cascade_explosion>(
		[&](const auto it) {
			if (const auto tr = it.find_viewing_transform(in.interpolation)) {
				const auto& cascade_def = it.template get<invariants::cascade_explosion>();

				const auto highlight_col = cascade_def.explosion.outer_ring_color;

				in.output.aabb_centered(
					in.cast_highlight_tex,
					tr->pos,
					highlight_col
				);
			}
		}
	);
}
