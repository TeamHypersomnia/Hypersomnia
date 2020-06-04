#pragma once
#include "game/modes/detail/flavour_getters.h"

template <class L>
transformr presentational_direct_attachment_offset(
	const L& logicals,
	const assets::image_id root_image_id,
	const assets::image_id attachment_image_id,
	const slot_function type
) {
	const auto anchors = [&]() {
		if (const auto image_id = attachment_image_id; image_id.is_set()) {
			return logicals.get_offsets(image_id).item;
		}

		return item_offsets();
	}();

	auto get_offsets_by_gun = [&]() {
		const auto image_id = root_image_id;

		if (image_id.is_set()) {
			return logicals.get_offsets(image_id).gun;
		}

		return gun_offsets();
	};

	transformi attachment_offset;
	transformi anchor;

	switch (type) {
		case slot_function::GUN_DETACHABLE_MAGAZINE: 
			attachment_offset = get_offsets_by_gun().detachable_magazine;
			anchor = anchors.attachment_anchor;
			break;

		case slot_function::GUN_CHAMBER: 
			attachment_offset = get_offsets_by_gun().chamber;
			anchor = anchors.attachment_anchor;
			break;

		case slot_function::GUN_CHAMBER_MAGAZINE: 
			attachment_offset = get_offsets_by_gun().chamber_magazine;
			anchor = anchors.attachment_anchor;
			break;

		default:
			attachment_offset = {};
			anchor = {};
			break;
	}

	return get_anchored_offset(attachment_offset, anchor);
}


template <class F>
void presentational_with_attachments(
	F&& callback,
	const cosmos& cosm,
	const item_flavour_id& flavour
) {
	const auto container_image = image_of(cosm, flavour);

	auto do_callback_for = [&](const slot_function type, const bool only_under) {
		const auto def = find_slot_def_of(cosm, flavour, type);

		if (def == nullptr) {
			return;
		}

		const bool should_process = def->makes_physical_connection();

		if (!should_process) {
			return;
		}

		const auto attachment_flavour = def->only_allow_flavour;
		const auto attachment_image = image_of(cosm, attachment_flavour);

		if (!should_process) {
			return;
		}

		if (only_under != def->draw_under_container) {
			return;
		}

		const auto presentational_offset = ::presentational_direct_attachment_offset(cosm.get_logical_assets(), container_image, attachment_image, type);

		callback(attachment_image, presentational_offset);
	};

	const auto presented_gun_attachments = std::array<slot_function, 5> {
		slot_function::GUN_CHAMBER,
		slot_function::GUN_DETACHABLE_MAGAZINE
	};

	for (const auto& s : presented_gun_attachments) {
		do_callback_for(s, true);
	}

	callback(container_image, transformr());

	for (const auto& s : presented_gun_attachments) {
		do_callback_for(s, false);
	}
}

template <class I>
ltrb aabb_of_game_image_with_attachments(
	const images_in_atlas_map& images_in_atlas,
	const cosmos& cosm,
	const I& flavour
) {
	ltrb result;

	auto get_entry = [&](const auto id) {
		return images_in_atlas.at(id).diffuse;
	};

	auto contain = [&](
		const auto& attachment_image,
		const auto& attachment_offset
	) {
		const auto& entry = get_entry(attachment_image);
		const auto& rotated_aabb = augs::calc_sprite_aabb(attachment_offset, entry.get_original_size());

		result.contain(rotated_aabb);
	};

	presentational_with_attachments(
		contain,
		cosm,
		flavour
	);

	return result;
}
