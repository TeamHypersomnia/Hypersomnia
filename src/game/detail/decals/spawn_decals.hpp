#pragma once
#include <optional>
#include <array>
#include <algorithm>
#include <limits>

#include <Box2D/Collision/Shapes/b2PolygonShape.h>

#include "augs/math/vec2.h"
#include "augs/math/camera_cone.h"
#include "augs/math/slide_rect_into_convex.h"
#include "augs/misc/randomization.h"
#include "augs/misc/constant_size_vector.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/create_entity.hpp"
#include "game/organization/all_entity_types.h"
#include "game/components/decal_component.h"
#include "game/components/sprite_component.h"
#include "game/common_state/common_assets.h"
#include "game/detail/decals/decal_geometry.h"
#include "game/detail/physics/calc_physical_material.hpp"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/visible_entities.hpp"
#include "game/inferred_caches/find_physics_cache.h"

/*
	Gunshot/melee decal sizing.
*/

/* The damage that spawns the gunshot decal at its original sprite size. */
inline constexpr real32 GUNSHOT_DECAL_BASELINE_DAMAGE = 16.f;

/*
	The damage-based mult is clamped to this range.
	A weapon's own invariants::gun::gunshot_decal_scale replaces it entirely.
*/
inline constexpr real32 MIN_GUNSHOT_DECAL_SCALE = 1.0f;
inline constexpr real32 MAX_GUNSHOT_DECAL_SCALE = 3.2f;

/*
	Surface decals are never shrunk below this size to fit their fixture;
	they are allowed to overhang slightly instead, so that a hit always leaves a mark.
*/
inline constexpr real32 MIN_SURFACE_DECAL_SIZE_PX = 2.f;

/*
	Gunshot/melee decal appearance.
*/

/* The inherited surface tint is brightened by this much so the decal stays visible, e.g. on dark glass. */
inline constexpr real32 SURFACE_DECAL_TINT_BRIGHTEN_MULT = 2.0f;

/*
	Stacking repeated hits in depth.
*/

/* When stacking decals in depth, move by this fraction of the decal's length per try. */
inline constexpr real32 DECAL_STACKING_STEP_MULT = 0.5f;

/*
	When looking for a free spot, existing decals are considered
	this much smaller - lets the decals overlap more densely.
*/
inline constexpr real32 DECAL_STACKING_NEIGHBOR_SIZE_MULT = 0.5f;

/*
	A safety cap only - with sane budgets, the binding constraint
	should be the bullet's actual reachable depth, not this.
*/
inline constexpr int MAX_DECAL_STACKING_TRIES = 32;

/* How many already-placed decals are considered when looking for a free spot. */
inline constexpr std::size_t MAX_NEARBY_DECALS_CONSIDERED = 64;

/*
	Once this many decals pile up in one spot, the oldest of them is deleted
	outright: more than this is just wasted fill rate over a mark
	that is already opaque, and the new one hides its disappearance anyway.
*/
inline constexpr std::size_t MAX_DECALS_PER_SPOT = 4;

/*
	Explosion decals.
*/

/* A force grenade explosion (88 damage) spawns the explosion decal at its original sprite size. */
inline constexpr real32 EXPLOSION_DECAL_BASELINE_DAMAGE = 88.f*3.0f;

/* Flash explosions deal negligible damage, so they leave a fixed-size decal instead. */
inline constexpr real32 FLASH_EXPLOSION_DECAL_SIZE_MULT = 0.5f;

/* Explosion decal opacity. */
inline const rgba EXPLOSION_DECAL_COLORIZE = rgba(255, 255, 255, 255);

/* The damage-based mult is clamped to this range. */
inline constexpr real32 MIN_EXPLOSION_DECAL_SCALE = 0.25f;
inline constexpr real32 MAX_EXPLOSION_DECAL_SCALE = 3.0f;

/* How many marks a single blast may leave on one surface entity. */
inline constexpr std::size_t MAX_EXPLOSION_DECALS_PER_SURFACE = 4;

/*
	A blast leaves one mark on a surface it hits within its radius,
	plus one more for every this much of the total weight the surface got.

	Weight = hit length in px, where each px counts as (1 - distance / blast radius)^2.
	So a px right at the blast counts fully, and a px at the edge of the radius not at all.
	E.g. a long flat wall right next to a blast of radius R gets 2R/3 in total.
*/
inline constexpr real32 EXPLOSION_SURFACE_WEIGHT_PER_DECAL = 40.f;

/* Sprite sizes are integral, so a thin decal must not round away to nothing. */
inline vec2i to_decal_sprite_size(const vec2 size) {
	return vec2i(
		std::max(1, static_cast<int>(size.x)),
		std::max(1, static_cast<int>(size.y))
	);
}

/* How far outside a fixture an impact point may land and still be attributed to it. */
inline constexpr real32 MAX_IMPACT_ATTRIBUTION_DISTANCE_PX = 10.f;

inline auto get_world_polygon_px(const b2Fixture& fixture, const si_scaling si) {
	augs::constant_size_vector<vec2, b2_maxPolygonVertices> result;

	const auto* const shape = fixture.GetShape();

	if (shape->GetType() != b2Shape::e_polygon) {
		return result;
	}

	const auto& poly = static_cast<const b2PolygonShape&>(*shape);
	const auto& xf = fixture.GetBody()->GetTransform();

	for (int v = 0; v < poly.GetVertexCount(); ++v) {
		result.push_back(si.get_pixels(static_cast<vec2>(b2Mul(xf, poly.GetVertex(v)))));
	}

	return result;
}

/*
	Finds the convex fixture of the surface entity whose boundary
	is the closest to the given impact point. Robust against impacts
	right at the corners, where a point-containment test would fail.
*/
template <class E>
const b2Fixture* find_fixture_of_impact(
	const E& surface_handle,
	const si_scaling si,
	const vec2 impact_point_px
) {
	const b2Fixture* best_fixture = nullptr;
	auto best_violation = MAX_IMPACT_ATTRIBUTION_DISTANCE_PX;

	if (const auto* const cache = ::find_colliders_cache(surface_handle)) {
		for (const auto& fp : cache->constructed_fixtures) {
			const auto* const f = fp.get();
			const auto polygon = ::get_world_polygon_px(*f, si);

			if (polygon.size() < 3) {
				continue;
			}

			const auto violation = augs::calc_max_edge_violation(
				polygon,
				augs::calc_centroid(polygon),
				impact_point_px
			);

			if (violation < best_violation) {
				best_violation = violation;
				best_fixture = f;
			}
		}
	}

	return best_fixture;
}

struct nearby_decal {
	entity_id id;
	vec2 pos;
	real32 len = 0.f;
};

using nearby_decals = augs::constant_size_vector<nearby_decal, MAX_NEARBY_DECALS_CONSIDERED>;

/*
	Collects the already-placed decals of the given layer around a box,
	so that a spot can be tested repeatedly without re-querying.
*/
template <render_layer Layer, class P>
void gather_nearby_decals(
	const cosmos& cosm,
	const vec2 box_center,
	const vec2 box_size,
	nearby_decals& output,
	P is_considered
) {
	output.clear();

	const auto clamped_size = vec2i(
		static_cast<int>(std::min(box_size.x, 100000.f)) + 2,
		static_cast<int>(std::min(box_size.y, 100000.f)) + 2
	);

	auto& visible = thread_local_visible_entities();

	/* Decals are non-physical, so the physical pass would be pure waste. */
	visible.acquire_non_physical({
		cosm,
		camera_cone(transformr(box_center), clamped_size),
		accuracy_type::EXACT,
		render_layer_filter::whitelist(Layer),
		tree_of_npo_filter::all()
	});

	visible.for_each<Layer>(cosm, [&](const auto& decal_handle) {
		decal_handle.template dispatch_on_having_all<components::decal>([&](const auto& typed_decal) {
			if (output.size() == output.max_size()) {
				return;
			}

			if (!is_considered(typed_decal)) {
				return;
			}

			const auto other_size = ::get_decal_size(typed_decal);

			output.push_back({
				typed_decal.get_id(),
				typed_decal.get_logic_transform().pos,
				std::max(other_size.x, other_size.y)
			});
		});
	});
}

inline bool decal_overlaps_spot(const nearby_decal& other, const vec2 at, const real32 len) {
	const auto overlap_radius = 0.4f * (len + other.len * DECAL_STACKING_NEIGHBOR_SIZE_MULT);

	return (other.pos - at).length_sq() < overlap_radius * overlap_radius;
}

/*
	Deletes the closest decal of an already crowded spot right away -
	the decal spawned on top of it covers its disappearance.
*/
inline void delete_closest_decal_if_crowded(
	const logic_step step,
	const nearby_decals& nearby,
	const vec2 at,
	const real32 len
) {
	std::size_t occupants = 0;

	auto closest = entity_id();
	auto closest_dist_sq = std::numeric_limits<real32>::max();
	auto closest_index = std::numeric_limits<unsigned>::max();

	for (const auto& other : nearby) {
		if (!::decal_overlaps_spot(other, at, len)) {
			continue;
		}

		++occupants;

		const auto dist_sq = (other.pos - at).length_sq();
		const auto index = other.id.raw.indirection_index;

		/* The index breaks ties so that the choice never depends on the query order. */
		if (dist_sq < closest_dist_sq || (dist_sq == closest_dist_sq && index < closest_index)) {
			closest_dist_sq = dist_sq;
			closest_index = index;
			closest = other.id;
		}
	}

	if (occupants < MAX_DECALS_PER_SPOT) {
		return;
	}

	if (const auto handle = step.get_cosmos()[closest]) {
		step.queue_deletion_of(handle, "Decal spot crowded");
	}
}

inline void queue_decal_creation(
	const logic_step step,
	const typed_entity_flavour_id<decal_decoration> flavour,
	const transformr decal_transform,
	const vec2i final_size,
	const entity_id spawned_by,
	const entity_id attached_to = entity_id(),
	const transformr attachment_offset = transformr(),
	const rgba colorize = white
) {
	cosmic::queue_create_entity(
		step,
		constrained_entity_flavour_id<invariants::decal>(flavour),
		[decal_transform, final_size, spawned_by, attached_to, attachment_offset, colorize](const auto typed_handle, auto& agg) {
			typed_handle.set_logic_transform(decal_transform);

			if (auto* const decal_state = agg.template find<components::decal>()) {
				decal_state->spawned_by = spawned_by;
				decal_state->attached_to = attached_to;
				decal_state->attachment_offset = attachment_offset;
			}

			if (auto* const geo = agg.template find<components::overridden_geo>()) {
				geo->size = final_size;
			}

			if (auto* const sprite = agg.template find<components::sprite>()) {
				sprite->colorize = colorize;
			}
		}
	);
}

/*
	The given variant list of the surface's material,
	or nullptr if the material defines no such decals.
*/
template <class S>
const material_decal_variants* find_material_decal_variants(
	const S& surface_handle,
	const material_decal_variants material_decals_def::* const variants_of_material
) {
	const auto material_id = ::calc_physical_material(surface_handle);

	if (!material_id.is_set()) {
		return nullptr;
	}

	const auto& material_decals = surface_handle.get_cosmos().get_common_assets().material_decals;
	const auto found_decals = material_decals.find(material_id);

	if (found_decals == material_decals.end()) {
		return nullptr;
	}

	const auto& variants = found_decals->second.*variants_of_material;

	if (variants.empty()) {
		return nullptr;
	}

	return &variants;
}

/*
	Spawns a gunshot, melee or explosion decal on the hit surface,
	picked from the given variant list of the surface's material.
	The decal slides along slide_dir into the hit fixture
	and is downscaled if there's not enough space there,
	so that it (almost) never sticks out of the convex fixture.

	When the target position already overlaps another surface decal,
	the decal is pushed deeper along the trajectory, step by step.
	get_max_stacking_depth_px is only invoked if that happens,
	and must yield the bullet's actual reachable depth in this material.

	Returns the final transform of the spawned decal, if any -
	e.g. so that the impact effects can be repositioned onto it.
*/
template <class S, class F>
std::optional<transformr> spawn_surface_impact_decal(
	const logic_step step,
	randomization& rng,
	const S& surface_handle,
	const b2Fixture* const fixture,
	const vec2 impact_point,
	const vec2 slide_dir,
	const real32 damage_amount,
	const real32 custom_decal_scale,
	const material_decal_variants material_decals_def::* const variants_of_material,
	F&& get_max_stacking_depth_px
) {
	if (fixture == nullptr || !(damage_amount > 0.f)) {
		return std::nullopt;
	}

	auto& cosm = step.get_cosmos();

	const auto* const found_variants = ::find_material_decal_variants(surface_handle, variants_of_material);

	if (found_variants == nullptr) {
		return std::nullopt;
	}

	const auto& variants = *found_variants;

	const auto flavour = variants[rng.randval(0, static_cast<int>(variants.size()) - 1)];

	if (!flavour.is_set()) {
		return std::nullopt;
	}

	const auto* const flavour_ptr = cosm.find_flavour(flavour);

	if (flavour_ptr == nullptr) {
		return std::nullopt;
	}

	const auto original_size = vec2(flavour_ptr->template get<invariants::sprite>().size);

	if (!(original_size.x > 0.f) || !(original_size.y > 0.f)) {
		/* A missing or unloaded decal image - would yield an infinite scale. */
		return std::nullopt;
	}

	/*
		A weapon may dictate its mark's scale outright;
		zero means it is derived from the damage instead.
	*/
	const auto size_mult =
		custom_decal_scale > 0.f ?
		custom_decal_scale :
		std::clamp(damage_amount / GUNSHOT_DECAL_BASELINE_DAMAGE, MIN_GUNSHOT_DECAL_SCALE, MAX_GUNSHOT_DECAL_SCALE)
	;

	const auto desired_size = original_size * size_mult * rng.randval(0.9f, 1.1f);

	if (!(desired_size.x > 0.f) || !(desired_size.y > 0.f)) {
		return std::nullopt;
	}
	const auto rotation = rng.randval(0.f, 360.f);

	const auto polygon = ::get_world_polygon_px(*fixture, cosm.get_si());

	if (polygon.empty()) {
		return std::nullopt;
	}

	const auto min_scale = std::min(1.f, MIN_SURFACE_DECAL_SIZE_PX / std::max(desired_size.x, desired_size.y));

	const auto fit = ::slide_rect_into_convex(
		polygon,
		impact_point,
		slide_dir,
		desired_size,
		rotation,
		min_scale
	);

	if (!fit.has_value()) {
		return std::nullopt;
	}

	const auto final_size_f = desired_size * fit->fitted_scale;
	const auto decal_len = std::max(final_size_f.x, final_size_f.y);

	auto final_center = fit->center;

	nearby_decals nearby;

	/*
		Stack repeated hits in depth: while the target position overlaps
		another surface decal, push deeper along the trajectory,
		as far as this bullet could penetrate this material.
	*/
	{
		const auto step_len = DECAL_STACKING_STEP_MULT * decal_len;

		auto gather_nearby = [&](const vec2 box_center, const vec2 box_size) {
			::gather_nearby_decals<render_layer::SURFACE_DECALS>(
				cosm,
				box_center,
				box_size,
				nearby,
				[](const auto&) { return true; }
			);
		};

		auto overlaps_any_nearby = [&](const vec2 at) {
			for (const auto& other : nearby) {
				if (::decal_overlaps_spot(other, at, decal_len)) {
					return true;
				}
			}

			return false;
		};

		/* A fresh surface is the common case: one small query and we are done. */
		gather_nearby(final_center, vec2::square(decal_len * 2));

		if (overlaps_any_nearby(final_center)) {
			const auto depth_budget = get_max_stacking_depth_px();

			if (depth_budget >= step_len) {
				/* Re-gather once over the whole corridor we may march through. */
				const auto march_end = final_center + fit->applied_slide_dir * depth_budget;
				const auto corridor = march_end - final_center;

				gather_nearby(
					(final_center + march_end) / 2,
					vec2(std::abs(corridor.x), std::abs(corridor.y)) + vec2::square(decal_len * 2)
				);

				auto depth_used = 0.f;

				for (int tries = 0; tries < MAX_DECAL_STACKING_TRIES; ++tries) {
					if (!overlaps_any_nearby(final_center)) {
						break;
					}

					if (depth_used + step_len > depth_budget) {
						break;
					}

					const auto next_center = final_center + fit->applied_slide_dir * step_len;

					if (!::rect_inside_convex(polygon, next_center, final_size_f, rotation, 0.5f)) {
						break;
					}

					final_center = next_center;
					depth_used += step_len;
				}
			}
		}
	}

	/*
		The march may have had to give up on a crowded spot -
		evict the mark we land on top of, so the pile stops growing.
	*/
	::delete_closest_decal_if_crowded(step, nearby, final_center, decal_len);

	const auto decal_transform = transformr(final_center, rotation);

	/*
		Inherit the surface's own tint, e.g. colored glass.
		Applied through colorize so it composes with the flavour's base color.
	*/
	const auto surface_color = [&]() {
		auto result = white;

		if (const auto* const surface_sprite = surface_handle.template find<invariants::sprite>()) {
			result = surface_sprite->color;
		}

		if (const auto* const surface_sprite_state = surface_handle.template find<components::sprite>()) {
			result *= surface_sprite_state->colorize;
		}

		result = augs::interp(result, white, 0.5f);

		/*
			Only the tint is inherited - the surface's own translucency
			(e.g. glass panes) must not fade the decal out.
		*/
		result.a = 255;

		return result;
	}();

	/*
		If the surface body can move, remember the decal's offset
		in the body's local space so that the decal can follow it.
	*/

	auto attached_to = entity_id();
	auto attachment_offset = transformr();

	if (fixture->GetBody()->GetType() != b2_staticBody) {
		if (const auto surface_transform = surface_handle.find_logic_transform()) {
			attached_to = surface_handle.get_id();
			attachment_offset = augs::get_relative_offset(*surface_transform, decal_transform);
		}
	}

	::queue_decal_creation(
		step,
		flavour,
		decal_transform,
		::to_decal_sprite_size(final_size_f),
		surface_handle.get_id(),
		attached_to,
		attachment_offset,
		surface_color
	);

	return decal_transform;
}

inline void spawn_explosion_decal(
	const logic_step step,
	const vec2 explosion_pos,
	const real32 size_mult,
	const entity_id subject
) {
	if (size_mult <= 0.f) {
		return;
	}

	auto& cosm = step.get_cosmos();
	const auto& assets = cosm.get_common_assets();

	auto rng = cosm.get_rng_for(subject);

	const std::array<typed_entity_flavour_id<decal_decoration>, 2> variants = {
		assets.explosion_decal_1,
		assets.explosion_decal_2
	};

	auto flavour = variants[rng.randval(0, 1)];

	if (!flavour.is_set()) {
		for (const auto& f : variants) {
			if (f.is_set()) {
				flavour = f;
				break;
			}
		}
	}

	if (!flavour.is_set()) {
		return;
	}

	const auto* const flavour_ptr = cosm.find_flavour(flavour);

	if (flavour_ptr == nullptr) {
		return;
	}

	const auto original_size = vec2(flavour_ptr->template get<invariants::sprite>().size);

	if (!(original_size.x > 0.f) || !(original_size.y > 0.f)) {
		return;
	}

	const auto final_size_mult = std::clamp(size_mult, MIN_EXPLOSION_DECAL_SCALE, MAX_EXPLOSION_DECAL_SCALE) * rng.randval(0.9f, 1.1f);
	const auto rotation = rng.randval(0.f, 360.f);

	/*
		Repeated blasts in one place would just pile up -
		evict the scorch mark we land on top of.
	*/
	{
		const auto final_size = original_size * final_size_mult;
		const auto decal_len = std::max(final_size.x, final_size.y);

		nearby_decals nearby;

		::gather_nearby_decals<render_layer::GROUND_DECALS>(
			cosm,
			explosion_pos,
			vec2::square(decal_len * 2),
			nearby,
			[](const auto& typed_decal) {
				return typed_decal.template get<invariants::decal>().is_explosion_decal;
			}
		);

		::delete_closest_decal_if_crowded(step, nearby, explosion_pos, decal_len);
	}

	::queue_decal_creation(
		step,
		flavour,
		transformr(explosion_pos, rotation),
		::to_decal_sprite_size(original_size * final_size_mult),
		subject,
		entity_id(),
		transformr(),
		EXPLOSION_DECAL_COLORIZE
	);
}
