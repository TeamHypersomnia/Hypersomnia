#pragma once
#include <algorithm>
#include <vector>
#include <array>
#include <optional>
#include <cmath>
#include "augs/misc/randomization.h"
#include "augs/templates/hash_templates.h"
#include "augs/math/polygon_queries.h"
#include "augs/algorithm/free_spot_search.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/decals/spawn_decals.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/messages/will_soon_be_deleted.h"
#include "game/messages/damage_message.h"
#include "game/components/sentience_component.h"
#include "game/components/explosive_component.h"
#include "game/components/melee_component.h"
#include "game/detail/decals/character_gunshot_drip.h"
#include "game/detail/sentience/lying_corpse_flip.h"
#include "game/detail/physics/stance_shape.h"

/*
	A bullet hole left on a character hit by a bullet, attached to it.

	On the hit it is only put where the bullet went in, as deep as it takes to be all within the colliders
	the character has then (see march_hole_into_outline). It keeps its place on the body, the stance the character
	was in (so its colliders then - see stance_shape.h) and the bullet's direction (in its rotation -
	see calc_character_hole_turn). A character keeps at most max_character_holes_v of them:
	the oldest go first, however big and wherever they are.

	Only corpses draw them (see owned_decals.h): a living character's arms move with every animation frame
	and the holes would float where they were - until it dies they are not even in the tree
	(see living_character_holes.h). A standing corpse draws those that are within its colliders.
	Once the character falls, its holes are laid out on the lying corpse - see move_character_decals_onto.

	Rolled like a wall's gunshot decal - variant and size from the damage (half of it).
*/

inline constexpr real32 character_gunshot_decal_size_mult_v = 0.5f;
inline constexpr std::size_t max_character_holes_v = 16;

/*
	A hole is drawn turned by the bullet's direction on the body and by an angle of its own,
	so that the holes shot from one side don't all look alike. The angle is taken from what the hole keeps -
	its size and variant - so that the bullet's direction can be had back when it is laid out on the lying corpse.
*/

inline real32 calc_character_hole_turn(const vec2i size, const raw_entity_flavour_id flavour) {
	return static_cast<real32>(augs::hash_multiple(size.x, size.y, flavour.indirection_index) % 360);
}

inline bool is_hole_within_outline(const stance_outline& outline, const vec2 p, const real32 radius) {
	return outline.empty() || (augs::is_within_polygon(outline, p) && augs::distance_to_polygon_outline(outline, p) >= radius);
}

/*
	A hole into the outline of a stance, like a wall's gunshot decal marching into its fixture:
	along the bullet's path - or for a grazing hit that never gets fully inside
	(the bullet is wide, so its middle may pass beside the body), from where it hit towards the middle of the body.
	Stays where it hit if it fits neither way.
*/

template <class G>
vec2 march_hole_into_outline(
	const stance_outline& outline,
	const vec2 from,
	const vec2 bullet_dir,
	const real32 radius,
	const real32 max_march,
	G&& is_taken
) {
	auto fits = [&](const vec2 p) {
		return ::is_hole_within_outline(outline, p, radius);
	};

	if (const auto along_path = augs::march_to_free_spot(from, bullet_dir, max_march, fits, is_taken)) {
		return *along_path;
	}

	if (from.is_nonzero()) {
		if (const auto towards_middle = augs::march_to_free_spot(from, -vec2(from).normalize(), max_march, fits, is_taken)) {
			return *towards_middle;
		}
	}

	return from;
}

/*
	Deleting a decal from here with queue_deletion_of may do nothing: a lying corpse calling this is mostly
	created in the step's last flush, after the queued deletions were taken.
	Marked for deletion with no time instead, decal_system deletes it in the next step.
*/

inline void delete_decal_next_step(components::decal& state) {
	state.marked_for_deletion = true;
	state.when_marked_for_deletion = {};
}

/*
	The holes and drips attached to a body, oldest first - one pass over all the decals,
	so that it depends on nothing inferred: neither on the tree (a living character's holes are not in it)
	nor on the order the tree would give them in.
*/

template <class C, class F>
std::vector<entity_id> find_character_decals_of(C& cosm, const entity_id owner, F&& is_considered) {
	struct found_decal {
		augs::stepped_timestamp when_born;
		entity_id id;
	};

	std::vector<found_decal> found;

	cosm.template for_each_having<components::decal>([&](const auto& typed_decal) {
		if (!typed_decal.template get<invariants::decal>().is_character_decal) {
			return;
		}

		auto& state = typed_decal.template get<components::decal>();

		if (state.attached_to == owner && is_considered(typed_decal, state)) {
			found.push_back({ typed_decal.when_born(), typed_decal.get_id() });
		}
	});

	std::sort(found.begin(), found.end(), [](const auto& a, const auto& b) {
		if (a.when_born.step != b.when_born.step) {
			return a.when_born.step < b.when_born.step;
		}

		return a.id < b.id;
	});

	std::vector<entity_id> result;
	result.reserve(found.size());

	for (const auto& f : found) {
		result.push_back(f.id);
	}

	return result;
}

template <class C>
std::vector<entity_id> find_character_holes_of(C& cosm, const entity_id owner) {
	return ::find_character_decals_of(cosm, owner, [&](const auto& typed_decal, const auto& state) {
		return
			!state.marked_for_deletion
			&& !::is_character_gunshot_drip(cosm.get_common_assets(), typed_decal.get_raw_flavour_id())
		;
	});
}

template <class H>
void spawn_character_gunshot_decal(
	const logic_step step,
	const H& character,
	const vec2 point,
	const vec2 impact_dir,
	const components::missile& missile,
	const real32 damage_amount
) {
	auto& cosm = step.get_cosmos();
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
	const auto decal_radius = vec2(size).length() / 2;

	/*
		A hit with no direction of its own goes towards the middle of the body.
	*/
	const auto bullet_dir = [&]() {
		if (impact_dir.is_nonzero()) {
			return vec2(impact_dir).normalize();
		}

		if (const auto to_middle = character_transform->pos - point; to_middle.is_nonzero()) {
			return vec2(to_middle).normalize();
		}

		return vec2::zero;
	}();

	const auto hit_stance = ::calc_collider_stance_id(character);
	const bool hit_stance_mirrored = character.only_secondary_holds_item();

	const auto local_point = vec2(point - character_transform->pos).rotate(-character_transform->rotation);
	const auto local_dir = vec2(bullet_dir).rotate(-character_transform->rotation);
	const auto local_bullet_degrees = local_dir.is_nonzero() ? local_dir.degrees() : 0.f;

	/*
		The bullet's middle (the point) is mostly still outside when it touches the body - so the hole goes in
		until all of it is within the colliders the character has now.
		Among the other holes it is laid out only at the fall - see move_character_decals_onto.
	*/
	const auto local_pos = ::march_hole_into_outline(
		::calc_stance_outline(character, hit_stance, hit_stance_mirrored),
		local_point,
		local_dir,
		decal_radius,
		character.get_logical_size().length() * 2,
		[](const vec2) { return false; }
	);

	const auto attachment_offset = transformr(
		local_pos,
		local_bullet_degrees + ::calc_character_hole_turn(size, flavour.raw)
	);

	const auto holes = ::find_character_holes_of(cosm, character.get_id());

	for (std::size_t i = 0; i + max_character_holes_v <= holes.size(); ++i) {
		cosm[holes[i]].template dispatch_on_having_all<components::decal>([](const auto& oldest) {
			::delete_decal_next_step(oldest.template get<components::decal>());
		});
	}

	if (auto* const sentience = character.template find<components::sentience>()) {
		sentience->newest_hole_offset = attachment_offset.pos;
	}

	const auto decal_transform = *character_transform * attachment_offset;
	const auto character_id = character.get_id();

	cosmic::queue_create_entity(
		step,
		constrained_entity_flavour_id<invariants::decal>(flavour),
		[decal_transform, size, character_id, attachment_offset, hit_stance, hit_stance_mirrored](const auto typed_handle, auto& agg) {
			typed_handle.set_logic_transform(decal_transform);

			if (auto* const state = agg.template find<components::decal>()) {
				state->spawned_by = character_id;
				state->attached_to = character_id;
				state->follows_attached = true;
				state->attachment_offset = attachment_offset;
				state->hit_stance = static_cast<uint8_t>(hit_stance);
				state->hit_stance_mirrored = hit_stance_mirrored;
			}

			if (auto* const geo = agg.template find<components::overridden_geo>()) {
				geo->size = size;
			}
		}
	);
}

/*
	A hit leaves a hole only when it was a bullet - not a rocket or the like, which explodes, nor a thrown knife
	(it comes through here as a missile too) - and it took health (not when the shield took all of it),
	not in the head, and didn't tear off a limb. Knives swung leave none, they don't come through here.
*/

struct character_before_hit {
	real32 health = 0.f;
	int limbs_detached = 0;

	explicit character_before_hit(const components::sentience& sentience) :
		health(sentience.get<health_meter_instance>().value),
		limbs_detached(sentience.num_limbs_detached())
	{}
};

template <class H, class M>
void leave_character_gunshot_decal(
	const logic_step step,
	const H& character,
	const M& typed_missile,
	const components::missile& missile,
	const character_before_hit before,
	const messages::damage_message& damage_msg,
	const vec2 point,
	const vec2 impact_dir
) {
	const auto& sentience = character.template get<components::sentience>();

	const bool lost_health = sentience.template get<health_meter_instance>().value < before.health;
	const bool tore_off_limb = sentience.num_limbs_detached() > before.limbs_detached;

	const bool explodes = [&]() {
		if (const auto* const explosive = typed_missile.template find<invariants::explosive>()) {
			return explosive->is_set();
		}

		return false;
	}();

	const bool thrown_melee = typed_missile.template find<invariants::melee>() != nullptr;

	if (lost_health && !tore_off_limb && !explodes && !thrown_melee && !damage_msg.origin.circumstances.headshot) {
		::spawn_character_gunshot_decal(
			step,
			character,
			point,
			impact_dir,
			missile,
			damage_msg.damage.base
		);
	}
}

/*
	The holes still attached to a character - e.g. one resurrected while its corpse still stood (test mode),
	whose holes would otherwise come back on its next corpse.
*/

inline void delete_holes_attached_to(const logic_step step, const entity_id character) {
	const auto& cosm = step.get_cosmos();

	for (const auto hole : ::find_character_decals_of(cosm, character, [](const auto&, const auto&) { return true; })) {
		step.queue_deletion_of(hole, "Hole of a resurrected character");
	}
}

/*
	A character that just died: its holes are put where it stands now and go into the tree,
	from where the standing corpse draws them - see living_character_holes.h.
*/

template <class H>
void reveal_holes_of_the_dead(const logic_step step, const H& character) {
	auto& cosm = step.get_cosmos();

	const auto character_transform = character.find_logic_transform();

	if (!character_transform.has_value()) {
		return;
	}

	for (const auto hole_id : ::find_character_decals_of(cosm, character.get_id(), [](const auto&, const auto&) { return true; })) {
		cosm[hole_id].template dispatch_on_having_all<components::decal>([&](const auto& hole) {
			hole.set_logic_transform(*character_transform * hole.template get<components::decal>().attachment_offset);
			cosmic::infer_caches_for(entity_handle(hole));
		});
	}
}

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

template <class E, class F, class G>
void spawn_lying_corpse_drip(
	const logic_step step,
	const E& corpse,
	const uint64_t drip_hash,
	const vec2 hole_local,
	const real32 hole_radius,
	F&& within_outline,
	G&& outline_reaches_into
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
		if (outline.empty()) {
			return true;
		}

		const auto p = to_shape_space(local);
		return augs::is_within_polygon(outline, p) && augs::distance_to_polygon_outline(outline, p) >= lying_corpse_decal_margin_v;
	};

	const auto previous = cosm[previous_owner];

	const auto layout = [&]() {
		if (previous) {
			if (const auto sentience_def = previous.template find<invariants::sentience>()) {
				return sentience_def->corpse_layout;
			}
		}

		return corpse_body_layout();
	}();

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
			for (const auto& other : laid_out) {
				if (::decal_overlaps_spot(other, at, len)) {
					return true;
				}
			}

			return false;
		};

		return ::march_hole_into_outline(standing_outline, from, bullet_dir, radius, max_standing_march, overlaps_laid_out);
	};

	const auto& being_deleted = step.get_queue<messages::will_soon_be_deleted>();

	auto is_being_deleted = [&](const entity_id id) {
		return std::any_of(being_deleted.begin(), being_deleted.end(), [id](const auto& d) { return d.subject == id; });
	};

	std::vector<entity_id> holes;

	for (const auto owned_id : ::find_character_decals_of(cosm, previous_owner, [](const auto&, const auto&) { return true; })) {
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
				for (const auto& other : laid_out_lying) {
					if (::decal_overlaps_spot(other, at, hole_len)) {
						return true;
					}
				}

				return false;
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
