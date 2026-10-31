#pragma once
#include <algorithm>
#include <vector>
#include <optional>
#include <cmath>
#include "augs/misc/randomization.h"
#include "augs/templates/hash_templates.h"
#include "augs/math/polygon_queries.h"
#include "augs/algorithm/free_spot_search.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/decals/spawn_decals.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/messages/damage_message.h"
#include "game/components/sentience_component.h"
#include "game/components/explosive_component.h"
#include "game/components/melee_component.h"
#include "game/detail/decals/character_gunshot_drip.h"
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
	Once the character falls, its holes are laid out on the lying corpse - see character_holes_layout.h.

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

template <class Polygon>
bool is_hole_within_outline(const Polygon& outline, const vec2 p, const real32 radius) {
	return outline.empty() || (augs::is_within_polygon(outline, p) && augs::distance_to_polygon_outline(outline, p) >= radius);
}

/*
	A hole into the outline of a stance, like a wall's gunshot decal marching into its fixture:
	along the bullet's path - or for a grazing hit that never gets fully inside
	(the bullet is wide, so its middle may pass beside the body), from where it hit towards the middle of the body.
	Stays where it hit if it fits neither way.
*/

template <class IsTaken>
vec2 march_hole_into_outline(
	const stance_outline& outline,
	const vec2 from,
	const vec2 bullet_dir,
	const real32 radius,
	const real32 max_march,
	IsTaken&& is_taken
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

template <class C, class IsConsidered>
std::vector<entity_id> find_character_decals_of(C& cosm, const entity_id owner, IsConsidered&& is_considered) {
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
std::vector<entity_id> find_all_character_decals_of(C& cosm, const entity_id owner) {
	return ::find_character_decals_of(cosm, owner, [](const auto&, const auto&) { return true; });
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
		Among the other holes it is laid out only at the fall - see character_holes_layout.h.
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

	const auto character_id = character.get_id();

	::queue_decal_creation(
		step,
		flavour,
		*character_transform * attachment_offset,
		size,
		character_id,
		character_id,
		true,
		attachment_offset,
		white,
		[hit_stance, hit_stance_mirrored](components::decal& state) {
			state.hit_stance = static_cast<uint8_t>(hit_stance);
			state.hit_stance_mirrored = hit_stance_mirrored;
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

	for (const auto hole : ::find_all_character_decals_of(cosm, character)) {
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

	for (const auto hole_id : ::find_all_character_decals_of(cosm, character.get_id())) {
		cosm[hole_id].template dispatch_on_having_all<components::decal>([&](const auto& hole) {
			hole.set_logic_transform(*character_transform * hole.template get<components::decal>().attachment_offset);
			cosmic::infer_caches_for(entity_handle(hole));
		});
	}
}
