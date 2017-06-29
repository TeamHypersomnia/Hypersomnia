#include "all.h"
#include "augs/graphics/drawers.h"
#include "augs/templates/visit_gettable.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sprite_component.h"

namespace rendering_scripts {
	void draw_cast_spells_highlights(
		augs::vertex_triangle_buffer& in,
		const interpolation_system& sys,
		const camera_cone cam,
		const cosmos& cosm,
		const double global_time_seconds
	) {
		const auto dt = cosm.get_fixed_delta();
		const auto& manager = get_assets_manager();

		cosm.for_each(
			processing_subjects::WITH_SENTIENCE,
			[&](const auto it) {
				const auto& sentience = it.get<components::sentience>();
				const auto casted_spell = sentience.currently_casted_spell;

				if (casted_spell.is_set()) {
					visit_gettable(
						sentience.spells,
						casted_spell,
						[&](const auto& spell){
							const auto highlight_amount = 1.f - (global_time_seconds - sentience.time_of_last_spell_cast.in_seconds(dt)) / 0.4f;

							if (highlight_amount > 0.f) {
								const auto spell_data = get_meta_of(spell, cosm.get_global_state().spells);

								components::sprite::drawing_input highlight(in);
								highlight.camera = cam;
								highlight.renderable_transform.pos = it.get_viewing_transform(sys).pos;

								auto highlight_col = spell_data.common.associated_color;
								highlight_col.a = static_cast<rgba_channel>(255 * highlight_amount);

								components::sprite spr;
								spr.set(
									assets::game_image_id::CAST_HIGHLIGHT, 
									manager[assets::game_image_id::CAST_HIGHLIGHT].get_size(),
									highlight_col
								);

								spr.draw(highlight);
							}
						}
					);
				}
			}
		);
	}
}
