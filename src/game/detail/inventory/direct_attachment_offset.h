#pragma once
#include "augs/math/transform.h"
#include "game/enums/slot_function.h"
#include "game/components/sprite_component.h"
#include "game/components/gun_component.h"
#include "game/detail/gun/gun_math.h"
#include "game/assets/image_offsets.h"
#include "game/components/torso_component.hpp"
#include "game/detail/frame_calculation.h"

inline transformr get_anchored_offset(
	const transformi attachment_offset,
	const transformi anchor
) {
	const auto rotation = attachment_offset.rotation;

	const auto old_center = attachment_offset.pos;
	const auto new_center = (old_center - anchor.pos).rotate(rotation, old_center);

	return { new_center, rotation - anchor.rotation };
}

struct attachment_offset_settings {
	bool consider_mag_rotations = false;
	bool consider_weapon_reloading = false;
	bool consider_weapon_shooting = false;

	static auto for_logic() {
		return attachment_offset_settings();
	}

	static auto for_rendering() {
		attachment_offset_settings output;
		output.consider_mag_rotations = true;
		output.consider_weapon_reloading = true;
		output.consider_weapon_shooting = true;
		return output;
	}
};

template <class A, class B, class C>
transformr direct_attachment_offset(
	const A& container, 
	const B& attachment,
	C get_offsets_by_torso,
	const attachment_offset_settings settings,
	const slot_function type
) {
	const auto& cosm = container.get_cosmos();
	const auto& logicals = cosm.get_logical_assets();

	const auto anchors = [&]() {
		if (const auto image_id = attachment.get_image_id(); image_id.is_set()) {
			return logicals.get_offsets(image_id).item;
		}

		return item_offsets();
	}();

	auto get_offsets_by_gun = [&]() {
		const auto image_id = [&]() {
			if (settings.consider_weapon_shooting) {
				if (const auto gun = container.template find<components::gun>()) {
					if (const auto gun_def = container.template find<invariants::gun>()) {
						if (const auto shoot_animation = logicals.find(gun_def->shoot_animation)) {
							if (const auto* const frame = ::find_shoot_frame(*gun_def, *gun, *shoot_animation, cosm)) {
								return frame->image_id;
							}
						}
					}
				}
			}

			return container.get_image_id();
		}();

		if (image_id.is_set()) {
			auto offsets = logicals.get_offsets(image_id).gun;

			if (settings.consider_mag_rotations) {
				if (const auto* const gun = container.template find<components::gun>()) {
					offsets.detachable_magazine.rotation += gun->magazine.rotation;
				}
			}

			return offsets;
		}

		return gun_offsets();
	};

	transformi attachment_offset;
	transformi anchor;

	const bool shall_flip = is_torso_attachment(type) && container.only_secondary_holds_item();

	auto get_actual_offsets_by_torso = [&]() {
		auto offsets = get_offsets_by_torso();

		if (shall_flip) {
			offsets.flip_vertically();
		}

		return offsets;
	};

	switch (type) {
		case slot_function::PRIMARY_HAND: 
			attachment_offset = get_actual_offsets_by_torso().primary_hand;
			anchor = anchors.hand_anchor;

			if (const auto h = container[slot_function::SECONDARY_HAND]; h && h.has_items()) {
				anchor.rotation = 0;
			}

			break;

		case slot_function::SECONDARY_HAND: 
			attachment_offset = get_actual_offsets_by_torso().secondary_hand;
			anchor = anchors.hand_anchor;

			if (const auto h = container[slot_function::PRIMARY_HAND]; h && h.has_items()) {
				anchor.rotation = 0;
			}

			break;

		case slot_function::BACK: 
			attachment_offset = get_actual_offsets_by_torso().back;
			anchor = anchors.back_anchor;
			break;

		case slot_function::OVER_BACK: 
			attachment_offset = get_actual_offsets_by_torso().back;
			anchor = anchors.back_anchor;
			break;

		case slot_function::SHOULDER: 
			attachment_offset = get_actual_offsets_by_torso().shoulder;
			anchor = anchors.shoulder_anchor;
			break;

		case slot_function::BELT: 
			attachment_offset = get_actual_offsets_by_torso().back;
			anchor = anchors.back_anchor;
			break;

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

		case slot_function::GUN_MUZZLE: 
			attachment_offset = ::calc_muzzle_transform(attachment, {}, get_offsets_by_gun().bullet_spawn).get_integerized();
			anchor = anchors.attachment_anchor;
			break;

		default:
			attachment_offset = {};
			anchor = {};
			break;
	}

	if (shall_flip) {
		anchor.flip_vertically();
	}

	if (settings.consider_weapon_reloading) {
		auto is_reloading_and_should_flip = [&](const auto& reloading_container) {
			if (::is_currently_reloading(reloading_container)) {
				if (const auto* const item = attachment.template find<invariants::item>()) {
					if (item->flip_when_reloading) {
						return true;
					}
				}
			}

			return false;
		};

		if (type == slot_function::PRIMARY_HAND || type == slot_function::SECONDARY_HAND) {
			if (is_reloading_and_should_flip(container)) {
				anchor.flip_vertically();
			}
		}
		else if (
			type == slot_function::GUN_DETACHABLE_MAGAZINE 
			|| type == slot_function::GUN_MUZZLE 
			|| type == slot_function::GUN_CHAMBER 
			|| type == slot_function::GUN_CHAMBER_MAGAZINE
		) {
			if (const auto slot = container.get_current_slot()) {
				if (is_reloading_and_should_flip(slot.get_container())) {
					anchor.flip_vertically();
					attachment_offset.flip_vertically();
				}
			}
		}
	}

	return get_anchored_offset(attachment_offset, anchor);
}

template <class A, class B>
transformr direct_attachment_offset(
	const A& container, 
	const B& attachment,
	const attachment_offset_settings settings,
	const slot_function type
) {
	const auto& cosm = container.get_cosmos();
	const auto& logicals = cosm.get_logical_assets();

	auto get_offsets_by_torso = [&]() {
		if (const auto* const torso = container.template find<invariants::torso>()) {
			const auto& stance = torso->calc_stance(container, container.get_wielded_items(), settings.consider_weapon_reloading);

			if (const auto fighter = container.template find<components::melee_fighter>()) {
				if (const auto frame = find_action_frame(stance, *fighter, logicals)) {
					return logicals.get_offsets(frame->image_id).torso;
				}
			}

			if (const auto* const anim = logicals.find(stance.carry)) {
				return logicals.get_offsets(anim->frames[0].image_id).torso;
			}
		}

		return torso_offsets();
	};

	return direct_attachment_offset(
		container, 
		attachment, 
		get_offsets_by_torso,
		settings,
		type
	);
}

