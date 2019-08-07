#include "rendering_scripts.h"
#include "augs/drawing/drawing.hpp"
#include "augs/templates/get_by_dynamic_id.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/sprite_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

#include "game/detail/sentience/pe_absorption.h"

void draw_explosion_body_highlights(const draw_explosion_body_highlights_input in) {
	const auto& cosm = in.cosm;

	const auto queried_camera_aabb = in.queried_cone.get_visible_world_rect_aabb();

	cosm.for_each_having<invariants::cascade_explosion>(
		[&](const auto& it) {
			if (const auto tr = it.find_viewing_transform(in.interpolation)) {
				const auto& cascade_def = it.template get<invariants::cascade_explosion>();

				const auto highlight_col = cascade_def.explosion.outer_ring_color;

				const auto highlight_ltrb = ltrb::center_and_size(tr->pos, in.cast_highlight_tex.get_original_size());

				if (!queried_camera_aabb.hover(highlight_ltrb)) {
					return;
				}

				in.output.aabb(
					in.cast_highlight_tex,
					highlight_ltrb,
					highlight_col
				);
			}
		}
	);
}
