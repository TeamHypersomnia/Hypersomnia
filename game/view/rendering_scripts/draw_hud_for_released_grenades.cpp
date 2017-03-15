#include "all.h"
#include "augs/graphics/drawers.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sprite_component.h"
#include "game/components/grenade_component.h"

namespace rendering_scripts {
	void draw_hud_for_released_grenades(
		augs::vertex_triangle_buffer& in,
		augs::special_buffer& in_special,
		const interpolation_system& sys,
		const camera_cone cam,
		const cosmos& cosm,
		const float global_time_seconds
	) {
		const auto dt = cosm.get_fixed_delta();

		cosm.for_each(
			processing_subjects::WITH_GRENADE,
			[&](const auto it) {
			const components::grenade& grenade = it.get<components::grenade>();

			if (grenade.when_explodes.was_set()) {
				const auto highlight_amount = 1.f - (
					(global_time_seconds - grenade.when_released.in_seconds(dt))
					/
					(grenade.when_explodes.in_seconds(dt) - grenade.when_released.in_seconds(dt))
				);

				if (highlight_amount > 0.f) {
					components::sprite::drawing_input highlight(in);
					highlight.camera = cam;
					highlight.renderable_transform.pos = it.get_viewing_transform(sys).pos;

					components::sprite spr;
					spr.set(assets::game_image_id::HUD_CIRCULAR_BAR_MEDIUM);
					//spr.draw(highlight);
				}
			}
		}
		);
	}
}
