#pragma once
#include "game/assets/ids/asset_ids.h"
#include "game/components/transform_component.h"
#include "game/components/sentience_component.h"
#include "game/assets/all_logical_assets.h"
#include "game/detail/inventory/direct_attachment_offset.h"

struct corpse_head_overlay_info {
	assets::image_id image_id;
	transformr world_transform;
	bool flipped = false;
};

template <class Handle, class F>
void for_each_corpse_head_overlay(
	const Handle& lying_corpse,
	const components::sentience& sentience,
	const invariants::sentience& sentience_def,
	const transformr& lying_viewing,
	const all_logical_assets& logicals,
	F&& callback
) {
	const auto corpse_image_id = lying_corpse.template get<invariants::sprite>().image_id;
	auto corpse_torso = logicals.get_offsets(corpse_image_id).torso;

	const bool corpse_flipped = [&]() {
		if (const auto flips = lying_corpse.calc_flip_flags()) {
			return flips->vertically;
		}
		return false;
	}();

	if (corpse_flipped) {
		corpse_torso.flip_vertically();
	}

	auto calc_overlay = [&](const assets::image_id overlay_image) -> corpse_head_overlay_info {
		auto head_anchor = logicals.get_offsets(overlay_image).item.head_anchor;

		if (corpse_flipped) {
			head_anchor.flip_vertically();
		}

		const auto target_offset = ::get_anchored_offset(corpse_torso.head, head_anchor);

		return {
			overlay_image,
			lying_viewing * target_offset,
			corpse_flipped
		};
	};

	const bool head_detached = sentience.detached.head.is_set();
	const bool was_headshot = sentience.knockout_origin.circumstances.headshot;
	const bool should_draw_head = !head_detached && !was_headshot;

	const auto head_or_splatter = should_draw_head
		? sentience_def.corpse_head_image
		: sentience_def.corpse_head_splatter_image
	;

	/*
		Shot in the head but killed by something else - a lesser splatter under the head still on it.
	*/
	if (should_draw_head && sentience.got_bloody_headshot && sentience_def.corpse_head_splatter_less_image.is_set()) {
		callback(calc_overlay(sentience_def.corpse_head_splatter_less_image));
	}

	if (head_or_splatter.is_set()) {
		callback(calc_overlay(head_or_splatter));
	}

	/*
		Body splatters: only for corpses blown up or cut into the body that kept everything -
		not headshotted, head attached, no limb torn off (whatever tore it off - one sword hit that takes a leg too).
		One for a blast, one for a cut - both if it got both.
	*/

	const bool kept_everything =
		!was_headshot
		&& !head_detached
		&& sentience.num_limbs_detached() == 0
	;

	auto draw_body_splatter = [&](const bool got_it, const assets::image_id image) {
		if (got_it && kept_everything && image.is_set()) {
			callback(calc_overlay(image));
		}
	};

	draw_body_splatter(sentience.got_body_blasted, sentience_def.corpse_body_blast_splatter_image);
	draw_body_splatter(sentience.got_body_cut, sentience_def.corpse_body_splatter_image);
}
