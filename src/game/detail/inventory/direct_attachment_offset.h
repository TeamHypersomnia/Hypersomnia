#pragma once
#include "augs/math/transform.h"
#include "game/enums/slot_function.h"
#include "game/components/sprite_component.h"
#include "game/components/gun_component.h"

template <class A, class B, class C>
transformr direct_attachment_offset(
	const A& container, 
	const B& attachment,
	C get_offsets_by_torso,
	const slot_function type
) {
	const auto& cosm = container.get_cosmos();
	const auto& logicals = cosm.get_logical_assets();

	const auto anchors = [&]() {
		if (const auto* const sprite = attachment.template find<invariants::sprite>()) {
			return logicals.get_offsets(sprite->image_id).item;
		}

		return item_offsets();
	}();

	auto get_offsets_by_gun = [&]() {
		if (const auto* const sprite = container.template find<invariants::sprite>()) {
			return logicals.get_offsets(sprite->image_id).gun;
		}

		return gun_offsets();
	};

	transformi attachment_offset;
	vec2i anchor;

	switch (type) {
		case slot_function::PRIMARY_HAND: 
		attachment_offset = get_offsets_by_torso().primary_hand;
		anchor = anchors.hand_anchor;
		break;

		case slot_function::SECONDARY_HAND: 
		attachment_offset = get_offsets_by_torso().secondary_hand;
		anchor = anchors.hand_anchor;
		break;

		case slot_function::SHOULDER: 
		attachment_offset = get_offsets_by_torso().back;
		anchor = anchors.back_anchor;
		break;

		case slot_function::GUN_DETACHABLE_MAGAZINE: 
		attachment_offset = get_offsets_by_gun().detachable_magazine;
		anchor = anchors.attachment_anchor;
		break;

		case slot_function::GUN_MUZZLE: 
		attachment_offset.pos = ::calc_muzzle_position(attachment, {});
		attachment_offset.rotation = 0;
		anchor = anchors.attachment_anchor;
		break;

		default:
		attachment_offset = {};
		anchor = {};
		break;
	}

	const auto rotation = attachment_offset.rotation;

	const auto old_center = attachment_offset.pos;
	const auto new_center = (old_center - anchor).rotate(rotation, old_center);

	return { new_center, rotation };
}

template <class A, class B>
transformr direct_attachment_offset(
	const A& container, 
	const B& attachment,
	const slot_function type
) {
	const auto& cosm = container.get_cosmos();
	const auto& logicals = cosm.get_logical_assets();

	auto get_offsets_by_torso = [&]() {
		if (const auto* const torso = container.template find<invariants::torso>()) {
			const auto& stance = torso->calc_stance(cosm, container.get_wielded_items());

			if (const auto* const anim = mapped_or_nullptr(logicals.torso_animations, stance.carry)) {
				return logicals.get_offsets(anim->frames[0].image_id).torso;
			}
		}

		return torso_offsets();
	};

	return direct_attachment_offset(
		container, 
		attachment, 
		get_offsets_by_torso,
		type
	);
}

