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
#include "game/detail/missile/penetration_path.h"

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

/* A weapon's own decal scale is clamped to this, so that content can never make it absurd. */
inline constexpr real32 MAX_CUSTOM_GUNSHOT_DECAL_SCALE = 10.f;

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
	Crash safety only, never meant to bind: the march already ends
	at a free spot, at the bullet's reachable depth, at the wall's end
	or after too long a run of spots the decal does not fit in.
*/
inline constexpr int MAX_DECAL_STACKING_STEPS_SAFETY_CAP = 1024;

/*
	While marching, spots where the decal would stick out of the wall are skipped,
	but for no longer than this many decal lengths in a row - a longer gap
	would be a visibly empty stretch of the tunnel.
*/
inline constexpr real32 MAX_DECAL_STACKING_SKIP_MULT = 1.0f;

/* How many already-placed decals are considered when looking for a free spot. */
inline constexpr std::size_t MAX_NEARBY_DECALS_CONSIDERED = 64;

/* How many fixtures of one wall are considered around a decal's march. */
inline constexpr std::size_t MAX_WALL_FIXTURES_CONSIDERED = 32;

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
inline constexpr real32 EXPLOSION_DECAL_BASELINE_DAMAGE = 88.f;

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
	/* Clamped, as casting an out-of-range float to int is undefined. */
	auto to_side = [](const real32 side) {
		return std::max(1, static_cast<int>(std::clamp(side, 1.f, 100000.f)));
	};

	return vec2i(to_side(size.x), to_side(size.y));
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
	const bool follows_attached = false,
	const transformr attachment_offset = transformr(),
	const rgba colorize = white
) {
	cosmic::queue_create_entity(
		step,
		constrained_entity_flavour_id<invariants::decal>(flavour),
		[decal_transform, final_size, spawned_by, attached_to, follows_attached, attachment_offset, colorize](const auto typed_handle, auto& agg) {
			typed_handle.set_logic_transform(decal_transform);

			if (auto* const decal_state = agg.template find<components::decal>()) {
				decal_state->spawned_by = spawned_by;
				decal_state->attached_to = attached_to;
				decal_state->follows_attached = follows_attached;
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

using wall_fixtures = augs::constant_size_vector<const b2Fixture*, MAX_WALL_FIXTURES_CONSIDERED>;

/*
	The fixtures around a box that form one wall with the entered fixture.
*/
inline wall_fixtures gather_wall_fixtures(
	const cosmos& cosm,
	const b2Fixture& entered,
	const vec2 box_center,
	const vec2 box_size
) {
	wall_fixtures result;

	const auto& physics = cosm.get_solvable_inferred().physics;

	physics.for_each_in_aabb(
		cosm.get_si(),
		box_center - box_size / 2,
		box_center + box_size / 2,
		filters[predefined_filter_type::PENETRATING_PROGRESS_QUERY],
		[&](const b2Fixture& f) {
			if (result.size() == result.max_size()) {
				return callback_result::ABORT;
			}

			if (::same_wall(entered, f)) {
				result.push_back(std::addressof(f));
			}

			return callback_result::CONTINUE;
		}
	);

	return result;
}

inline const b2Fixture* find_wall_fixture_at(
	const wall_fixtures& wall,
	const si_scaling si,
	const vec2 point
) {
	const auto point_meters = b2Vec2(si.get_meters(point));

	for (const auto* const f : wall) {
		if (f->TestPoint(point_meters)) {
			return f;
		}
	}

	return nullptr;
}

/*
	Whether a rotated decal lies entirely within the wall, however many fixtures it spans.
	Edge midpoints are tested too, so that a concave junction of fixtures is not missed.
*/
inline bool decal_fits_in_wall(
	const wall_fixtures& wall,
	const si_scaling si,
	const vec2 center,
	const vec2 size,
	const real32 rotation
) {
	const auto corners = augs::make_rotated_corners(size, rotation);

	for (std::size_t i = 0; i < corners.size(); ++i) {
		const auto a = center + corners[i];
		const auto b = center + corners[(i + 1) % corners.size()];

		if (::find_wall_fixture_at(wall, si, a) == nullptr) {
			return false;
		}

		if (::find_wall_fixture_at(wall, si, (a + b) / 2) == nullptr) {
			return false;
		}
	}

	return true;
}

/*
	Spawns a gunshot, melee or explosion decal on the hit surface,
	picked from the given variant list of the surface's material.
	The decal slides along slide_dir into the hit fixture
	and is downscaled if there's not enough space there,
	so that it (almost) never sticks out of the convex fixture.

	When the target position already overlaps another surface decal,
	the decal is pushed deeper along the trajectory, step by step -
	also into the other fixtures of the same wall, taking on
	the owner and the material of the fixture it ends up in.
	get_max_stacking_depth_px is only invoked if that happens,
	and must yield how deep the bullet actually gets within this wall.

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

	const auto variant_index = static_cast<std::size_t>(rng.randval(0, static_cast<int>(variants.size()) - 1));
	const auto flavour = variants[variant_index];

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
		std::min(custom_decal_scale, MAX_CUSTOM_GUNSHOT_DECAL_SCALE) :
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

	auto final_size_f = desired_size * fit->fitted_scale;
	auto final_center = fit->center;
	const b2Fixture* placed_fixture = fixture;

	if (fit->fitted_scale < 1.f) {
		/*
			Shrunk to fit the entered fixture alone - but the wall may well go on
			into a neighbouring fixture, e.g. at the corner of a tile. Take the nearest
			spot along the slide where the full-size decal fits into the wall instead.
		*/
		if (const auto full_fit = ::slide_rect_into_convex(polygon, impact_point, slide_dir, desired_size, rotation, 1.f)) {
			const auto full_len = std::max(desired_size.x, desired_size.y);
			const auto search_len = MAX_DECAL_STACKING_SKIP_MULT * full_len;
			const auto search_step = DECAL_STACKING_STEP_MULT * full_len;
			const auto search_dir = full_fit->applied_slide_dir;

			const auto wall = ::gather_wall_fixtures(
				cosm,
				*fixture,
				full_fit->center + search_dir * (search_len / 2),
				vec2::square(full_len * 2 + search_len)
			);

			for (auto dist = 0.f; dist <= search_len; dist += search_step) {
				const auto candidate = full_fit->center + search_dir * dist;

				if (::decal_fits_in_wall(wall, cosm.get_si(), candidate, desired_size, rotation)) {
					final_size_f = desired_size;
					final_center = candidate;

					if (const auto* const containing = ::find_wall_fixture_at(wall, cosm.get_si(), candidate)) {
						placed_fixture = containing;
					}

					break;
				}
			}
		}
	}

	const auto decal_len = std::max(final_size_f.x, final_size_f.y);


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
				/*
					Always along the bullet's line - the fit's inward push on grazing hits
					only serves to get the first decal inside the wall.
				*/
				const auto march_dir = slide_dir;
				const auto march_end = final_center + march_dir * depth_budget;
				const auto corridor = march_end - final_center;
				const auto corridor_center = (final_center + march_end) / 2;
				const auto corridor_size = vec2(std::abs(corridor.x), std::abs(corridor.y)) + vec2::square(decal_len * 2);

				/* Gather once for the whole corridor we may march through. */
				gather_nearby(corridor_center, corridor_size);
				const auto wall = ::gather_wall_fixtures(cosm, *fixture, corridor_center, corridor_size);

				const auto max_skip = MAX_DECAL_STACKING_SKIP_MULT * decal_len;

				auto candidate = final_center;
				auto skipped = 0.f;

				for (int steps = 0; steps < MAX_DECAL_STACKING_STEPS_SAFETY_CAP; ++steps) {
					if (!overlaps_any_nearby(final_center)) {
						break;
					}

					const auto next_candidate = candidate + march_dir * step_len;

					/* The budget is how deep the bullet gets, as measured from where it hit. */
					if ((next_candidate - impact_point).dot(march_dir) > depth_budget) {
						break;
					}

					candidate = next_candidate;

					if (!::decal_fits_in_wall(wall, cosm.get_si(), candidate, final_size_f, rotation)) {
						skipped += step_len;

						if (skipped > max_skip) {
							break;
						}

						continue;
					}

					skipped = 0.f;
					final_center = candidate;
				}

				if (const auto* const containing = ::find_wall_fixture_at(wall, cosm.get_si(), final_center)) {
					placed_fixture = containing;
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
		The march may have carried the decal into another fixture of the wall,
		possibly of another entity and material.
	*/
	const auto placed_surface = cosm[placed_fixture->GetUserData()];

	if (placed_surface.dead()) {
		return std::nullopt;
	}

	const auto placed_flavour = [&]() {
		if (placed_surface.get_id() == surface_handle.get_id()) {
			return flavour;
		}

		if (const auto* const placed_variants = ::find_material_decal_variants(placed_surface, variants_of_material)) {
			const auto candidate = (*placed_variants)[variant_index % placed_variants->size()];

			if (candidate.is_set() && cosm.find_flavour(candidate) != nullptr) {
				return candidate;
			}
		}

		return flavour;
	}();

	/*
		Inherit the surface's own tint, e.g. colored glass.
		Applied through colorize so it composes with the flavour's base color.
	*/
	const auto surface_color = [&]() {
		auto result = white;

		if (const auto* const surface_sprite = placed_surface.template find<invariants::sprite>()) {
			result = surface_sprite->color;
		}

		if (const auto* const surface_sprite_state = placed_surface.template find<components::sprite>()) {
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
		Always attach to the surface so that the decal is deleted together with it
		(e.g. a static tutorial wall removed once the level is cleared).

		If the surface body can move, also remember the decal's offset
		in the body's local space so that the decal can follow it.
	*/

	const auto attached_to = placed_surface.get_id();
	auto follows_attached = false;
	auto attachment_offset = transformr();

	if (placed_fixture->GetBody()->GetType() != b2_staticBody) {
		if (const auto surface_transform = placed_surface.find_logic_transform()) {
			follows_attached = true;
			attachment_offset = augs::get_relative_offset(*surface_transform, decal_transform);
		}
	}

	::queue_decal_creation(
		step,
		placed_flavour,
		decal_transform,
		::to_decal_sprite_size(final_size_f),
		placed_surface.get_id(),
		attached_to,
		follows_attached,
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
		false,
		transformr(),
		EXPLOSION_DECAL_COLORIZE
	);
}
