#pragma once
#include <algorithm>
#include <limits>
#include <vector>
#include <optional>
#include "augs/misc/randomization.h"
#include "augs/templates/hash_templates.h"
#include "augs/misc/convex_partitioned_shape.hpp"
#include "augs/math/convex_polygon.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/decals/spawn_decals.hpp"
#include "game/detail/decals/character_gunshot_drip.h"
#include "game/inferred_caches/find_physics_cache.h"

/*
	A bullet hole left on a character hit by a bullet, attached to it so that it moves along.
	Not drawn while the character stands - its arms move with every animation frame and the holes would float
	where they were. It is there to know where it was hit: once the character falls, its holes move onto
	the lying corpse, which draws them.

	Rolled like a wall's gunshot decal - variant, rotation and size from the damage (half of it) -
	and like it stacked in depth: a hole landing on another one of this character marches on along the bullet's path,
	evicting the closest one of an already crowded spot.

	See move_character_decals_onto.
*/

inline constexpr real32 character_gunshot_decal_size_mult_v = 0.5f;

template <class H>
void spawn_character_gunshot_decal(
	const logic_step step,
	const H& character,
	const vec2 point,
	const vec2 impact_dir,
	const components::missile& missile,
	const real32 damage_amount
) {
	const auto& cosm = step.get_cosmos();
	const auto& assets = cosm.get_common_assets();

	const auto character_transform = character.find_logic_transform();

	if (!character_transform.has_value()) {
		return;
	}

	/*
		Seeded by the bullet and the character, like spawn_gunshot_decal_of_impact.
	*/
	auto rng = randomization(augs::hash_multiple(
		missile.rng_seed,
		character.get_id().raw.indirection_index
	));

	const auto flavour = rng.randval(0, 1) == 0 ? assets.character_gunshot_decal_1 : assets.character_gunshot_decal_2;

	if (!flavour.is_set()) {
		return;
	}

	const auto original_size = cosm.on_flavour(flavour, [](const auto& f) {
		return vec2(f.template get<invariants::sprite>().size);
	});

	const auto size_mult =
		character_gunshot_decal_size_mult_v
		* ::calc_gunshot_decal_size_mult(damage_amount, missile.decal_scale_of_sender)
		* rng.randval(0.9f, 1.1f)
	;

	const auto size = vec2i(original_size * size_mult);
	const auto rotation = rng.randval(0.f, 360.f);

	const auto decal_radius = vec2(size).length() / 2;
	const auto decal_len = std::max(size.x, size.y);
	const auto si = cosm.get_si();

	/*
		The character's colliders as they are now - they depend on its stance (e.g. the pistol one is not convex,
		a standing corpse without an arm is shorter on that side), so they are taken from the physics as they are.
	*/
	std::vector<std::vector<vec2>> body_polygons;

	if (const auto* const cache = ::find_colliders_cache(character)) {
		for (const auto& fp : cache->constructed_fixtures) {
			auto polygon = ::get_world_polygon_px(*fp.get(), si);

			if (polygon.size() >= 3) {
				body_polygons.emplace_back(polygon.begin(), polygon.end());
			}
		}
	}

	auto within_body = [&](const vec2 p) {
		for (const auto& polygon : body_polygons) {
			if (augs::is_within_convex_polygon(polygon, p)) {
				return true;
			}
		}

		return false;
	};

	/*
		The whole hole within the body: its middle and the points around it at its radius.
	*/
	auto fits = [&](const vec2 p) {
		if (body_polygons.empty()) {
			return true;
		}

		if (!within_body(p)) {
			return false;
		}

		for (int i = 0; i < 8; ++i) {
			if (!within_body(p + vec2::from_degrees(45.f * i) * decal_radius)) {
				return false;
			}
		}

		return true;
	};

	nearby_decals nearby;

	::gather_nearby_decals<render_layer::CHARACTER_DECALS>(
		cosm,
		character_transform->pos,
		character.get_logical_size() * 2,
		nearby,
		[&](const auto& typed_decal) {
			return typed_decal.template get<components::decal>().attached_to == character.get_id();
		}
	);

	auto overlaps_any_nearby = [&](const vec2 at) {
		for (const auto& other : nearby) {
			if (::decal_overlaps_spot(other, at, static_cast<real32>(decal_len))) {
				return true;
			}
		}

		return false;
	};

	/*
		Like a wall's gunshot decal marching into its fixture: along the bullet's path,
		first until the whole hole is within the body, then on past the holes already there,
		as long as it stays within it. A grazing hit that never gets fully inside
		marches from where it hit towards the middle of the body instead.
	*/
	const auto world_pos = [&]() {
		const auto max_march = character.get_logical_size().length() * 2;

		auto march = [&](const vec2 from, const vec2 dir) -> std::optional<vec2> {
			std::optional<vec2> last_fit;

			for (real32 marched = 0.f; marched <= max_march; marched += 1.f) {
				const auto candidate = from + dir * marched;

				if (!fits(candidate)) {
					if (last_fit.has_value()) {
						break;
					}

					continue;
				}

				last_fit = candidate;

				if (!overlaps_any_nearby(candidate)) {
					return candidate;
				}
			}

			return last_fit;
		};

		if (impact_dir.is_nonzero()) {
			if (const auto along_path = march(point, vec2(impact_dir).normalize())) {
				return *along_path;
			}
		}

		if (const auto to_middle = character_transform->pos - point; to_middle.is_nonzero()) {
			if (const auto towards_middle = march(point, vec2(to_middle).normalize())) {
				return *towards_middle;
			}
		}

		return character_transform->pos;
	}();

	const auto decal_transform = transformr(world_pos, rotation);

	::delete_closest_decal_if_crowded(step, nearby, decal_transform.pos, static_cast<real32>(decal_len));

	const auto attachment_offset = augs::get_relative_offset(*character_transform, decal_transform);

	if (auto* const sentience = character.template find<components::sentience>()) {
		sentience->newest_hole_offset = attachment_offset.pos;
	}

	::queue_decal_creation(
		step,
		flavour,
		decal_transform,
		size,
		character.get_id(),
		character.get_id(),
		true,
		attachment_offset
	);
}

/*
	Where the character's newest hole is now (none: zero offset) - the blood dripping from the body runs from it.
*/

template <class E>
std::optional<vec2> find_newest_character_hole_pos(const E& character, const components::sentience& sentience) {
	if (sentience.newest_hole_offset.is_zero()) {
		return std::nullopt;
	}

	if (const auto character_transform = character.find_logic_transform()) {
		return (*character_transform * transformr(sentience.newest_hole_offset)).pos;
	}

	return std::nullopt;
}

/*
	When a character falls, its holes move onto the lying corpse - head at -x, the arms along its sides
	from the shoulders (-x) to the hands (around 0), the legs at +x.

	A hole keeps its side of the body: standing +y is the same side of the body as lying +y.
	A flip of either sprite is not counted: an intact body is symmetric, and with one arm torn off
	both sprites are flipped alike, so the missing arm is on the same side.

	Along the lying body it lands where it was on the standing one:
	- in the torso (up to lying_corpse_standing_torso_front_v forward): around the middle of the chest,
	  the back of the standing torso closer to the neck, its front closer to the belly -
	  the shoulders (from standing_shoulder_from_v across) count as the arms when they are torn off,
	- ahead of it, on an arm of whatever stance: on that side's arm, from the elbow to the hand
	  as far forward as it was towards the stance's reach.
	Never on the legs.

	Under each hole on a lying corpse, blood runs from it towards the legs, as it would have on the standing body -
	but only if the whole drip lies on the corpse: its outline is not convex, so it is checked against
	the sprite's own outline, not the convex parts.
	Drips are made anew whenever the holes move.

	When a lying corpse is replaced by one with fewer limbs, the holes keep their places on the body -
	mirrored across its length if the new sprite is flipped the other way.

	Either way a hole has to end up within the corpse's outline, at least lying_corpse_decal_margin_v from its edge -
	otherwise it is moved towards its shoulder (on an arm) or the chest a little, and if that is not enough,
	it was on a limb the corpse no longer has and goes with it.
*/

inline constexpr real32 lying_corpse_standing_torso_front_v = 22.f;
inline constexpr real32 lying_corpse_chest_v = -35.f;
inline constexpr real32 lying_corpse_chest_depth_spread_v = 12.f;
inline constexpr real32 lying_corpse_chest_jitter_v = 6.f;
inline constexpr real32 lying_corpse_arm_from_v = -30.f;
inline constexpr real32 lying_corpse_arm_to_v = 2.f;
inline constexpr real32 lying_corpse_arm_across_v = 30.f;
inline constexpr real32 lying_corpse_decal_margin_v = 3.f;
inline constexpr real32 lying_corpse_drip_overlap_v = 1.f;
inline constexpr int lying_corpse_fallen_max_pull_v = 64;
inline constexpr int lying_corpse_replaced_max_pull_v = 6;
inline constexpr real32 standing_shoulders_half_width_v = 41.f;
inline constexpr real32 standing_shoulder_from_v = 30.f;
inline constexpr real32 lying_corpse_shoulders_half_width_v = 38.f;

/*
	The drip hangs from the edge of the hole on the side of the legs (overlapping it a little) and runs towards them -
	the lying corpse's +x, where its image's down is turned. It is drawn under the holes.
	The whole of it has to be on the corpse, so that none of it hangs in the air.
	All in the corpse's space.
*/

template <class E, class F, class G>
void spawn_lying_corpse_drip(
	const logic_step step,
	const E& corpse,
	const entity_id hole,
	const vec2 hole_local,
	const real32 hole_radius,
	F&& within_outline,
	G&& outline_reaches_into
) {
	const auto& cosm = step.get_cosmos();
	const auto& assets = cosm.get_common_assets();

	const auto variant = augs::hash_multiple(hole.raw.indirection_index, hole.raw.version) % 3;

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

	const auto drip_outline = std::vector<vec2> {
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
	const bool from_standing,
	const bool mirrored = false
) {
	auto& cosm = step.get_cosmos();

	const auto owner_transform = new_owner.get_logic_transform();
	const auto& shape = cosm.get_logical_assets().get_offsets(new_owner.get_image_id()).non_standard_shape;

	const bool flipped = [&]() {
		if (const auto flips = new_owner.calc_flip_flags()) {
			return flips->vertically;
		}

		return false;
	}();

	auto distance_to_outline = [&](const vec2 p) {
		const auto& verts = shape.source_polygon;
		auto closest = std::numeric_limits<real32>::max();

		for (std::size_t i = 0; i < verts.size(); ++i) {
			const auto a = vec2(verts[i]);
			const auto b = vec2(verts[(i + 1) % verts.size()]);
			const auto ab = b - a;
			const auto len_sq = ab.length_sq();
			const auto t = len_sq > 0.f ? std::clamp((p - a).dot(ab) / len_sq, 0.f, 1.f) : 0.f;

			closest = std::min(closest, (p - (a + ab * t)).length());
		}

		return closest;
	};

	/*
		In the shape's own space - mirrored if the corpse is drawn flipped.
	*/
	auto within_outline = [&](vec2 local) {
		if (flipped) {
			local.y = -local.y;
		}

		const auto& verts = shape.source_polygon;
		bool inside = false;

		for (std::size_t i = 0, j = verts.size() - 1; i < verts.size(); j = i++) {
			const auto a = vec2(verts[i]);
			const auto b = vec2(verts[j]);

			if ((a.y > local.y) != (b.y > local.y) && local.x < (b.x - a.x) * (local.y - a.y) / (b.y - a.y) + a.x) {
				inside = !inside;
			}
		}

		return inside;
	};

	/*
		Whether a corner of the outline is within a convex polygon of the corpse's space.
	*/
	auto outline_reaches_into = [&](const std::vector<vec2>& convex) {
		for (const auto& v : shape.source_polygon) {
			auto local = vec2(v);

			if (flipped) {
				local.y = -local.y;
			}

			if (augs::is_within_convex_polygon(convex, local)) {
				return true;
			}
		}

		return false;
	};

	auto fits = [&](vec2 local) {
		if (shape.empty()) {
			return true;
		}

		if (flipped) {
			local.y = -local.y;
		}

		bool inside = false;

		shape.for_each_convex([&](const auto& convex) {
			inside = inside || augs::is_within_convex_polygon(convex, local);
		});

		return inside && distance_to_outline(local) >= lying_corpse_decal_margin_v;
	};

	const auto previous = cosm[previous_owner];

	const auto previous_transform = previous ? previous.get_logic_transform() : transformr();

	/*
		Across, the standing shoulders (whatever the stance's sprite) onto the lying ones.
	*/
	const auto across_scale = lying_corpse_shoulders_half_width_v / standing_shoulders_half_width_v;

	/*
		How far forward the stance reaches (e.g. the hands with a rifle), from the standing colliders.
	*/
	const auto standing_reach = [&]() {
		auto reach = lying_corpse_standing_torso_front_v;

		if (previous) {
			if (const auto* const cache = ::find_colliders_cache(previous)) {
				for (const auto& fp : cache->constructed_fixtures) {
					for (const auto& v : ::get_world_polygon_px(*fp.get(), cosm.get_si())) {
						reach = std::max(reach, vec2(v - previous_transform.pos).rotate(-previous_transform.rotation).x);
					}
				}
			}
		}

		return reach;
	}();

	/*
		Only the decals around the body are looked at - the new one lies where the previous one stood or lay.
		Sorted, as the tree gives them in an order that depends on how it was built (it is inferred),
		and the order decides the ids of the drips made.
	*/
	const auto previous_size = previous ? previous.get_logical_size().length() : 0.f;
	const auto query_size = vec2::square(2 * std::max(previous_size, new_owner.get_logical_size().length()));

	std::vector<entity_id> owned;

	::for_each_decal_in_box<render_layer::CHARACTER_DECALS>(cosm, owner_transform.pos, query_size, [&](const auto& typed_decal) {
		if (typed_decal.template get<components::decal>().attached_to == previous_owner) {
			owned.push_back(typed_decal.get_id());
		}
	});

	std::sort(owned.begin(), owned.end());

	for (const auto owned_id : owned) {
		cosm[owned_id].template dispatch_on_having_all<components::decal>([&](const auto& decal_handle) {
			auto& state = decal_handle.template get<components::decal>();

			if (::is_character_gunshot_drip(cosm.get_common_assets(), decal_handle.get_raw_flavour_id())) {
				step.queue_deletion_of(decal_handle, "Drip made anew");
				return;
			}

			const auto decal_transform = decal_handle.get_logic_transform();

			/*
				Where a hole that does not fit is pulled to, at most max_pull - for an arm its shoulder,
				so it stays on the arm instead of crossing into the body.
				A hole that does not fit even then was on a limb this corpse no longer has, and goes with it.
			*/
			auto anchor = vec2(lying_corpse_chest_v, 0.f);
			auto max_pull = lying_corpse_replaced_max_pull_v;

			auto local = [&]() {
				if (from_standing && previous) {
					max_pull = lying_corpse_fallen_max_pull_v;

					const auto standing_offset = vec2(state.attachment_offset.pos);

					const auto id = decal_handle.get_id();
					const auto id_hash = augs::hash_multiple(id.raw.indirection_index, id.raw.version);

					const auto side = [&]() {
						if (std::abs(standing_offset.y) >= 1.f) {
							return standing_offset.y > 0.f ? 1.f : -1.f;
						}

						return id_hash % 2 == 0 ? 1.f : -1.f;
					}();

					const auto shoulder = vec2(lying_corpse_arm_from_v, side * lying_corpse_arm_across_v);

					if (standing_offset.x <= lying_corpse_standing_torso_front_v) {
						if (std::abs(standing_offset.y) >= standing_shoulder_from_v) {
							anchor = shoulder;
						}

						const auto depth = std::clamp(standing_offset.x / lying_corpse_standing_torso_front_v, -1.f, 1.f);
						const auto jitter = (static_cast<real32>(id_hash % 1001) / 500.f - 1.f) * lying_corpse_chest_jitter_v;

						return vec2(
							lying_corpse_chest_v + depth * lying_corpse_chest_depth_spread_v + jitter,
							standing_offset.y * across_scale
						);
					}

					const auto t = std::clamp(
						(standing_offset.x - lying_corpse_standing_torso_front_v) / std::max(1.f, standing_reach - lying_corpse_standing_torso_front_v),
						0.f,
						1.f
					);

					anchor = shoulder;

					return vec2(
						augs::interp(lying_corpse_arm_from_v, lying_corpse_arm_to_v, t),
						side * lying_corpse_arm_across_v
					);
				}

				auto on_previous = vec2(state.attachment_offset.pos);

				if (mirrored) {
					on_previous.y = -on_previous.y;
				}

				return on_previous;
			}();

			for (int i = 0; i < max_pull && !fits(local) && local != anchor; ++i) {
				const auto to_anchor = anchor - local;
				local += vec2(to_anchor).set_length(std::min(1.f, to_anchor.length()));
			}

			if (!fits(local)) {
				step.queue_deletion_of(decal_handle, "Hole on a missing limb");
				return;
			}

			const auto new_transform = transformr(
				owner_transform.pos + vec2(local).rotate(owner_transform.rotation),
				decal_transform.rotation
			);

			state.attached_to = new_owner.get_id();
			state.attachment_offset = augs::get_relative_offset(owner_transform, new_transform);

			decal_handle.set_logic_transform(new_transform);
			cosmic::infer_caches_for(entity_handle(decal_handle));

			const auto hole_size = decal_handle.get_logical_size();
			const auto hole_radius = std::min(hole_size.x, hole_size.y) / 2;

			::spawn_lying_corpse_drip(step, new_owner, decal_handle.get_id(), local, hole_radius, within_outline, outline_reaches_into);
		});
	}
}
