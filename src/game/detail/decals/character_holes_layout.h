#pragma once
#include <array>
#include <vector>
#include <optional>
#include <cmath>
#include "game/messages/will_soon_be_deleted.h"
#include "game/detail/decals/character_gunshot_decal.h"
#include "game/detail/sentience/lying_corpse_flip.h"

/*
	When a character falls, its holes move onto the lying corpse - head at -x, the arms along its sides
	from the shoulders (-x) to the hands (around 0), the legs at +x. Where the body's parts are in the sprites
	is the character's corpse_body_layout.

	First each is put where it would be on the standing body as it was when hit, oldest first -
	like a wall's gunshot decal marching into its fixture: along the bullet's path, first until the whole hole
	is within the colliders of the stance the character was in then, then on past the holes put there before,
	as long as it stays within them. A grazing hit that never gets fully inside marches from where it hit
	towards the middle of the body instead, and one that fits neither way stays where it hit.

	A hole keeps its side of the body: standing +y is the same side of the body as lying +y.
	A flip of either sprite is not counted: an intact body is symmetric, and with one arm torn off
	both sprites are flipped alike, so the missing arm is on the same side.

	Along the lying body it lands where it was on the standing one:
	- in the middle of the torso (up to standing_torso_front forward, less than standing_shoulder_from across):
	  around the middle of the chest, the back of the standing torso closer to the neck, its front closer to the belly,
	- on the side of the torso: on that side's arm, from the shoulder at the torso's front to the wrist at its back -
	  or only to the arm's middle when the stance it was hit in holds the arms ahead (reaches past the torso),
	- ahead of the torso: on that side's arm, from its middle to the wrist as far forward as it was
	  towards the stance's reach.
	Never on the hands nor the legs.

	Under each hole on a lying corpse, blood runs from it towards the legs, as it would have on the standing body -
	but only if the whole drip lies on the corpse: its outline is not convex, so it is checked against
	the sprite's own outline, not the convex parts.
	Drips are made anew whenever the holes move.

	When a lying corpse is replaced by one with fewer limbs, the holes keep their places on the body -
	mirrored across its length if the new sprite is flipped the other way.

	Either way a hole has to end up within the corpse's outline, at least lying_corpse_decal_margin_v from its edge -
	otherwise it is moved towards its shoulder (on an arm) or the chest a little, and if that is not enough,
	it was on a limb the corpse no longer has and goes with it.
	At the fall, one that lands on a hole laid out there before goes to the nearest free spot around it:
	many spots of the standing body end up on one of the lying corpse (e.g. its whole side on the shoulder).
	The outline is the sprite's own (source_polygon) - the colliders are of a simpler one (see its convex_partition).
*/

inline constexpr real32 lying_corpse_chest_depth_spread_v = 12.f;
inline constexpr real32 lying_corpse_chest_jitter_v = 6.f;
inline constexpr real32 lying_corpse_decal_margin_v = 3.f;
inline constexpr real32 lying_corpse_drip_overlap_v = 1.f;
inline constexpr int lying_corpse_fallen_max_pull_v = 64;
inline constexpr int lying_corpse_replaced_max_pull_v = 6;
inline constexpr real32 lying_corpse_hole_spread_radius_mult_v = 3.f;
inline constexpr real32 lying_corpse_arm_width_mult_v = 0.5f;

/*
	The drip hangs from the edge of the hole on the side of the legs (overlapping it a little) and runs towards them -
	the lying corpse's +x, where its image's down is turned. It is drawn under the holes.
	The whole of it has to be on the corpse, so that none of it hangs in the air.
	All in the corpse's space.
*/

template <class E, class WithinOutline, class ReachesInto>
void spawn_lying_corpse_drip(
	const logic_step step,
	const E& corpse,
	const uint64_t drip_hash,
	const vec2 hole_local,
	const real32 hole_radius,
	WithinOutline&& within_outline,
	ReachesInto&& outline_reaches_into
) {
	const auto& cosm = step.get_cosmos();
	const auto& assets = cosm.get_common_assets();

	const auto variant = drip_hash % 3;

	const auto flavour =
		variant == 0 ? assets.character_gunshot_drip_1 :
		variant == 1 ? assets.character_gunshot_drip_2 :
		assets.character_gunshot_drip_3
	;

	if (!flavour.is_set()) {
		return;
	}

	const auto size = cosm.on_flavour(flavour, [](const auto& f) {
		return vec2(f.template get<invariants::sprite>().size);
	});

	const auto corpse_transform = corpse.get_logic_transform();

	const auto drip_offset = transformr(
		hole_local + vec2(std::max(0.f, hole_radius - lying_corpse_drip_overlap_v) + size.y / 2, 0.f),
		-90.f
	);

	/*
		Its outline: full width at the hole, narrowing towards its end (the third one ends in two strands).
	*/
	const auto from_x = drip_offset.pos.x - size.y / 2;
	const auto to_x = drip_offset.pos.x + size.y / 2;
	const auto end_half_width = variant == 2 ? 3.5f : 1.f;
	const auto y = drip_offset.pos.y;

	const auto drip_outline = std::array<vec2, 4> {
		vec2(from_x, y - size.x / 2),
		vec2(to_x, y - end_half_width),
		vec2(to_x, y + end_half_width),
		vec2(from_x, y + size.x / 2)
	};

	for (const auto& corner : drip_outline) {
		if (!within_outline(corner)) {
			return;
		}
	}

	if (outline_reaches_into(drip_outline)) {
		return;
	}

	const auto drip_transform = corpse_transform * drip_offset;

	::queue_decal_creation(
		step,
		flavour,
		drip_transform,
		vec2i(size),
		corpse.get_id(),
		corpse.get_id(),
		true,
		drip_offset
	);
}

template <class E>
void move_character_decals_onto(
	const logic_step step,
	const entity_id previous_owner,
	const E& new_owner,
	const corpse_body_layout& layout,
	const bool from_standing,
	const bool mirrored = false
) {
	auto& cosm = step.get_cosmos();

	const auto owner_transform = new_owner.get_logic_transform();
	const auto& outline = cosm.get_logical_assets().get_offsets(new_owner.get_image_id()).non_standard_shape.source_polygon;

	const bool flipped = ::is_lying_corpse_flipped(new_owner);

	/*
		From the corpse's space into the shape's own - mirrored if the corpse is drawn flipped.
		Mirroring is its own inverse, so it goes the other way too.
	*/
	auto to_shape_space = [flipped](vec2 p) {
		if (flipped) {
			p.y = -p.y;
		}

		return p;
	};

	auto within_outline = [&](const vec2 local) {
		return augs::is_within_polygon(outline, to_shape_space(local));
	};

	/*
		Whether a corner of the outline is within a convex polygon of the corpse's space.
	*/
	auto outline_reaches_into = [&](const auto& convex) {
		for (const auto& v : outline) {
			if (augs::is_within_convex_polygon(convex, to_shape_space(vec2(v)))) {
				return true;
			}
		}

		return false;
	};

	auto fits = [&](const vec2 local) {
		return ::is_hole_within_outline(outline, to_shape_space(local), lying_corpse_decal_margin_v);
	};

	const auto previous = cosm[previous_owner];

	/*
		Across, the standing shoulders (whatever the stance's sprite) onto the lying ones.
	*/
	const auto across_scale = layout.lying_shoulders_half_width / layout.standing_shoulders_half_width;

	/*
		The outlines of the stances the holes were made in - each computed once.
	*/
	constexpr auto num_stances = static_cast<std::size_t>(item_holding_stance::COUNT);
	std::array<std::optional<stance_outline>, num_stances * 2> hit_outlines;

	auto get_hit_outline = [&](const components::decal& state) -> const stance_outline& {
		const auto stance_index = std::min(static_cast<std::size_t>(state.hit_stance), num_stances - 1);
		auto& cached = hit_outlines[stance_index * 2 + (state.hit_stance_mirrored ? 1 : 0)];

		if (!cached.has_value()) {
			cached = ::calc_stance_outline(previous, static_cast<item_holding_stance>(stance_index), state.hit_stance_mirrored);
		}

		return *cached;
	};

	const auto max_standing_march = previous ? previous.get_logical_size().length() * 2 : 0.f;

	/*
		The holes already put on the standing body and on the lying corpse, each in its space.
	*/
	nearby_decals laid_out;
	nearby_decals laid_out_lying;

	auto put_on_standing_body = [&](const stance_outline& standing_outline, const vec2 from, const vec2 bullet_dir, const real32 radius, const real32 len) {
		auto overlaps_laid_out = [&](const vec2 at) {
			return ::decal_overlaps_any(laid_out, at, len);
		};

		return ::march_hole_into_outline(standing_outline, from, bullet_dir, radius, max_standing_march, overlaps_laid_out);
	};

	const auto& being_deleted = step.get_queue<messages::will_soon_be_deleted>();

	auto is_being_deleted = [&](const entity_id id) {
		return std::any_of(being_deleted.begin(), being_deleted.end(), [id](const auto& d) { return d.subject == id; });
	};

	std::vector<entity_id> holes;

	for (const auto owned_id : ::find_all_character_decals_of(cosm, previous_owner)) {
		cosm[owned_id].template dispatch_on_having_all<components::decal>([&](const auto& decal_handle) {
			auto& state = decal_handle.template get<components::decal>();

			if (::is_character_gunshot_drip(cosm.get_common_assets(), decal_handle.get_raw_flavour_id())) {
				::delete_decal_next_step(state);
				return;
			}

			/*
				A hole already on its way out stays behind, without a drip.
			*/
			if (state.marked_for_deletion || is_being_deleted(owned_id)) {
				return;
			}

			holes.push_back(owned_id);
		});
	}

	/*
		Of a character that took more hits in its last step than it keeps holes, only the newest ones.
	*/
	const auto first_kept = holes.size() > max_character_holes_v ? holes.size() - max_character_holes_v : 0;

	for (std::size_t i = 0; i < holes.size(); ++i) {
		cosm[holes[i]].template dispatch_on_having_all<components::decal>([&](const auto& decal_handle) {
			auto& state = decal_handle.template get<components::decal>();

			if (i < first_kept) {
				::delete_decal_next_step(state);
				return;
			}

			const auto decal_transform = decal_handle.get_logic_transform();

			/*
				Rolled from what the bullet rolled for the hole - not from its entity id, which depends on whatever else
				was created before it, and so may differ after a misprediction.
				The drip from its size and variant only, which no move changes - so a replaced corpse keeps its drips;
				where it lands at the fall also from its rotation on the standing body (rounded - it is recomputed).
			*/
			const auto hole_size = vec2i(decal_handle.get_logical_size());
			const auto flavour = decal_handle.get_raw_flavour_id();
			const auto drip_hash = augs::hash_multiple(hole_size.x, hole_size.y, flavour.indirection_index);

			const auto hole_hash = augs::hash_multiple(
				static_cast<int>(std::round(state.attachment_offset.rotation)),
				hole_size.x,
				hole_size.y
			);

			const auto hole_radius = static_cast<real32>(std::min(hole_size.x, hole_size.y)) / 2;

			/*
				Where a hole that does not fit is pulled to, at most max_pull - for an arm its shoulder,
				so it stays on the arm instead of crossing into the body.
				A hole that does not fit even then was on a limb this corpse no longer has, and goes with it.
			*/
			auto anchor = vec2(layout.lying_chest, 0.f);
			auto max_pull = lying_corpse_replaced_max_pull_v;

			auto local = [&]() {
				if (from_standing && previous) {
					max_pull = lying_corpse_fallen_max_pull_v;

					const auto& hit_outline = get_hit_outline(state);
					const auto bullet_degrees = state.attachment_offset.rotation - ::calc_character_hole_turn(hole_size, flavour);

					const auto standing_offset = put_on_standing_body(
						hit_outline,
						vec2(state.attachment_offset.pos),
						vec2::from_degrees(bullet_degrees),
						vec2(hole_size).length() / 2,
						static_cast<real32>(std::max(hole_size.x, hole_size.y))
					);

					if (laid_out.size() < laid_out.max_size()) {
						laid_out.push_back({ decal_handle.get_id(), standing_offset, static_cast<real32>(std::max(hole_size.x, hole_size.y)) });
					}

					/*
						How far forward the stance reaches (e.g. the hands with a rifle).
					*/
					const auto standing_reach = [&]() {
						auto reach = layout.standing_torso_front;

						for (const auto& v : hit_outline) {
							reach = std::max(reach, v.x);
						}

						return reach;
					}();

					const auto side = [&]() {
						if (std::abs(standing_offset.y) >= 1.f) {
							return standing_offset.y > 0.f ? 1.f : -1.f;
						}

						return hole_hash % 2 == 0 ? 1.f : -1.f;
					}();

					const auto shoulder = vec2(layout.lying_arm_from, side * layout.lying_arm_across);
					const auto arm_middle = (layout.lying_arm_from + layout.lying_arm_to) / 2;
					const bool arms_ahead = standing_reach > layout.standing_torso_front;

					auto on_arm = [&](const real32 along) {
						anchor = shoulder;
						return vec2(along, side * layout.lying_arm_across);
					};

					if (standing_offset.x <= layout.standing_torso_front) {
						if (const auto across = std::abs(standing_offset.y); across >= layout.standing_shoulder_from) {
							/*
								The side of the torso is the arm along it: from the shoulder at its front (what sticks out
								the most of a body facing the shooter) to the wrist at its back - or only to the arm's middle
								when the stance holds the arms ahead. So the holes pushed deeper by the shots from the front
								go down the arm. Its outer edge is the arm's outer side.
							*/
							const auto front = layout.standing_torso_front;
							const auto depth = std::clamp((front - standing_offset.x) / (2 * front), 0.f, 1.f);
							const auto arm_end = arms_ahead ? arm_middle : layout.lying_arm_to;
							const auto band_middle = (layout.standing_shoulder_from + layout.standing_shoulders_half_width) / 2;

							auto result = on_arm(augs::interp(layout.lying_arm_from, arm_end, depth));
							result.y += side * (across - band_middle) * lying_corpse_arm_width_mult_v;
							return result;
						}

						const auto depth = std::clamp(standing_offset.x / layout.standing_torso_front, -1.f, 1.f);
						const auto jitter = (static_cast<real32>(hole_hash % 1001) / 500.f - 1.f) * lying_corpse_chest_jitter_v;

						return vec2(
							layout.lying_chest + depth * lying_corpse_chest_depth_spread_v + jitter,
							standing_offset.y * across_scale
						);
					}

					/*
						Ahead of the torso: the forearm, from the arm's middle to the wrist as far forward as it was.
					*/
					const auto t = std::clamp(
						(standing_offset.x - layout.standing_torso_front) / std::max(1.f, standing_reach - layout.standing_torso_front),
						0.f,
						1.f
					);

					return on_arm(augs::interp(arm_middle, layout.lying_arm_to, t));
				}

				auto on_previous = vec2(state.attachment_offset.pos);

				if (mirrored) {
					on_previous.y = -on_previous.y;
				}

				return on_previous;
			}();

			for (int pull = 0; pull < max_pull && !fits(local) && local != anchor; ++pull) {
				const auto to_anchor = anchor - local;
				local += vec2(to_anchor).set_length(std::min(1.f, to_anchor.length()));
			}

			if (!fits(local)) {
				::delete_decal_next_step(state);
				return;
			}

			/*
				At the fall, a hole landing on one laid out on the lying corpse before goes to the nearest free spot around -
				e.g. the hits along one edge of the body all land on its shoulder, and would cover each other there.
				A replaced corpse's holes keep their places.
			*/
			const auto hole_len = static_cast<real32>(std::max(hole_size.x, hole_size.y));

			auto is_taken = [&](const vec2 at) {
				return ::decal_overlaps_any(laid_out_lying, at, hole_len);
			};

			if (from_standing && is_taken(local)) {
				const auto spread_radius = lying_corpse_hole_spread_radius_mult_v * hole_len;
				const auto start_degrees = static_cast<real32>(hole_hash % 360);

				if (const auto free_spot = augs::find_free_spot_around(local, spread_radius, start_degrees, fits, is_taken)) {
					local = *free_spot;
				}
			}

			if (laid_out_lying.size() < laid_out_lying.max_size()) {
				laid_out_lying.push_back({ decal_handle.get_id(), local, hole_len });
			}

			const auto new_transform = transformr(
				owner_transform.pos + vec2(local).rotate(owner_transform.rotation),
				decal_transform.rotation
			);

			state.attached_to = new_owner.get_id();
			state.attachment_offset = augs::get_relative_offset(owner_transform, new_transform);

			decal_handle.set_logic_transform(new_transform);
			cosmic::infer_caches_for(entity_handle(decal_handle));

			::spawn_lying_corpse_drip(step, new_owner, drip_hash, local, hole_radius, within_outline, outline_reaches_into);
		});
	}
}
