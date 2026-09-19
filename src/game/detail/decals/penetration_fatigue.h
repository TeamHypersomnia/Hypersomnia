#pragma once
#include <cstddef>
#include <cstdint>
#include <array>
#include <bit>
#include <algorithm>

#include "augs/math/vec2.h"
#include "augs/math/camera_cone.h"
#include "augs/math/segment_obb.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/components/decal_component.h"
#include "game/detail/decals/decal_geometry.h"
#include "game/detail/visible_entities.hpp"

/*
	Material fatigue: stretches of a fixture already covered by gunshot decals
	are cheaper to penetrate by this factor.
*/
inline constexpr real32 DECAL_FATIGUE_RELIEF = 0.5f;

/*
	A fully fatigued material must still cost something,
	and a covered px must still consume some of the gift.
*/
static_assert(DECAL_FATIGUE_RELIEF > 0.f && DECAL_FATIGUE_RELIEF < 1.f);

/*
	The total power the fatigue may gift to a single bullet over its lifetime,
	as a multiple of its base penetration distance.
	1.0 = a bullet can penetrate at most as if it had 2x its base penetration.
	0.0 disables the whole mechanic, including its query costs.
*/
inline constexpr real32 DECAL_FATIGUE_MAX_GIFT_MULT = 1.0f;

/*
	The coverage of a segment is quantized into this many buckets.
	A bitmask makes the result independent of the order in which
	the decals arrive from the spatial query, and imposes no limit
	on how many decals may contribute.
*/
inline constexpr std::size_t DECAL_COVERAGE_BUCKETS = 256;

using decal_coverage_mask = std::array<uint64_t, DECAL_COVERAGE_BUCKETS / 64>;

inline bool is_coverage_bucket_set(const decal_coverage_mask& mask, const std::size_t i) {
	return 0 != (mask[i / 64] & (uint64_t(1) << (i % 64)));
}

inline std::size_t count_coverage_buckets(const decal_coverage_mask& mask) {
	std::size_t result = 0;

	for (const auto word : mask) {
		result += static_cast<std::size_t>(std::popcount(word));
	}

	return result;
}

/* How much of the fatigue gift is still available to this bullet. */
inline real32 calc_remaining_fatigue_gift(
	const real32 base_penetration_distance,
	const real32 gift_already_used
) {
	return std::max(0.f, DECAL_FATIGUE_MAX_GIFT_MULT * base_penetration_distance - gift_already_used);
}

/*
	Marks the buckets of the [p1, p2] segment covered by the surface decals
	of the given entity. Bucket boundaries are rounded to the nearest bucket,
	with a guarantee that no intersecting decal is lost entirely.

	Only decals born before only_born_before_step count -
	a bullet may only benefit from the fatigue that existed
	when it was fired, never from its own or its volley's decals.

	Uses a thread_local visible_entities buffer. Safe in the logic step
	and in the view, but it must never be called from inside
	another visible_entities::for_each on the same thread.
*/
inline decal_coverage_mask calc_decal_coverage_mask(
	const cosmos& cosm,
	const entity_id surface_owner,
	const vec2 p1,
	const vec2 p2,
	const unsigned only_born_before_step
) {
	decal_coverage_mask mask = {};

	if (!surface_owner.is_set()) {
		return mask;
	}

	const auto segment = p2 - p1;

	if (!(segment.length_sq() > 0.f)) {
		return mask;
	}

	const auto query_center = (p1 + p2) / 2;

	const auto query_size = vec2i(
		static_cast<int>(std::min(std::abs(segment.x), 100000.f)) + 4,
		static_cast<int>(std::min(std::abs(segment.y), 100000.f)) + 4
	);

	auto& visible = thread_local_visible_entities();

	/* Decals are non-physical, so the physical pass would be pure waste. */
	visible.acquire_non_physical({
		cosm,
		camera_cone(transformr(query_center), query_size),
		accuracy_type::EXACT,
		render_layer_filter::whitelist(render_layer::SURFACE_DECALS),
		tree_of_npo_filter::all()
	});

	visible.for_each<render_layer::SURFACE_DECALS>(cosm, [&](const auto& decal_handle) {
		decal_handle.template dispatch_on_having_all<components::decal>([&](const auto& typed_decal) {
			/* Only the decals of this very fixture's owner may fatigue it. */
			if (typed_decal.template get<components::decal>().spawned_by != surface_owner) {
				return;
			}

			if (!(typed_decal.when_born().step < only_born_before_step)) {
				return;
			}

			const auto decal_transform = typed_decal.get_logic_transform();

			const auto hit = ::segment_obb_intersection(
				p1,
				p2,
				decal_transform.pos,
				::get_decal_size(typed_decal) / 2,
				decal_transform.rotation
			);

			if (!hit.has_value()) {
				return;
			}

			constexpr auto n = static_cast<real32>(DECAL_COVERAGE_BUCKETS);

			auto first = static_cast<std::size_t>(std::clamp(hit->first * n + 0.5f, 0.f, n - 1));
			auto last = static_cast<std::size_t>(std::clamp(hit->second * n + 0.5f, 0.f, n));

			if (last <= first) {
				/* Thinner than a bucket - still worth exactly one. */
				last = first + 1;
			}

			for (auto i = first; i < std::min(last, DECAL_COVERAGE_BUCKETS); ++i) {
				mask[i / 64] |= uint64_t(1) << (i % 64);
			}
		});
	});

	return mask;
}

/*
	Returns the total length (px) of the [p1, p2] segment covered
	by the surface decals spawned on the given entity.
*/
inline real32 calc_decal_covered_length_px(
	const cosmos& cosm,
	const entity_id surface_owner,
	const vec2 p1,
	const vec2 p2,
	const unsigned only_born_before_step
) {
	const auto segment_length = (p2 - p1).length();

	if (!(segment_length > 0.f)) {
		return 0.f;
	}

	const auto mask = ::calc_decal_coverage_mask(cosm, surface_owner, p1, p2, only_born_before_step);
	const auto buckets = ::count_coverage_buckets(mask);

	return segment_length * static_cast<real32>(buckets) / static_cast<real32>(DECAL_COVERAGE_BUCKETS);
}

struct penetration_cost_result {
	real32 cost = 0.f;

	/* How many px of penetration power the fatigue gifted on this stretch. */
	real32 gifted = 0.f;
};

/*
	The penetration cost of the [p1, p2] stretch inside a fixture,
	discounted by the decals ("material fatigue") already covering it.

	Because the covered length is clipped to this very stretch
	and DECAL_FATIGUE_RELIEF < 1, a single fixture can never refund
	more than it would itself consume.

	max_gift additionally caps the discount - the caller tracks
	how much power the fatigue has gifted to this bullet in total.
*/
inline penetration_cost_result calc_penetration_cost_px(
	const cosmos& cosm,
	const entity_id surface_owner,
	const vec2 p1,
	const vec2 p2,
	const real32 penetrability,
	const real32 max_gift,
	const unsigned only_born_before_step
) {
	/* Negated comparisons so that a NaN takes the safe branch. */
	if (!(penetrability > 0.f)) {
		return { std::numeric_limits<real32>::max(), 0.f };
	}

	const auto full_cost = (p2 - p1).length() / penetrability;

	if (!(max_gift > 0.f)) {
		return { full_cost, 0.f };
	}

	const auto covered = ::calc_decal_covered_length_px(cosm, surface_owner, p1, p2, only_born_before_step);
	const auto gifted = std::min(DECAL_FATIGUE_RELIEF * covered / penetrability, max_gift);

	return { full_cost - gifted, gifted };
}

struct penetration_reach_result {
	/* How far into the material (px) the bullet gets. */
	real32 reach = 0.f;

	/* How much of the gift that took. */
	real32 gifted = 0.f;
};

/*
	Walks the material from entry_point along dir through the CURRENT
	decal coverage and returns how far the bullet actually gets
	before running out of power.

	Uses exactly the same rates as calc_penetration_cost_px:
	a clean px costs 1/penetrability of power; a covered px costs
	(1 - relief)/penetrability and consumes relief/penetrability of the gift.

	dir must be normalized.
*/
inline penetration_reach_result calc_penetration_reach_px(
	const cosmos& cosm,
	const entity_id surface_owner,
	const vec2 entry_point,
	const vec2 dir,
	const real32 power,
	const real32 max_gift,
	const real32 penetrability,
	const unsigned only_born_before_step
) {
	/* Negated comparisons so that a NaN takes the safe branch. */
	if (!(power > 0.f) || !(penetrability > 0.f)) {
		return {};
	}

	if (!(max_gift > 0.f)) {
		return { power * penetrability, 0.f };
	}

	/* The theoretical maximum, when everything ahead is covered. */
	const auto max_len = std::min(
		power / (1.f - DECAL_FATIGUE_RELIEF),
		power + max_gift
	) * penetrability;

	const auto mask = ::calc_decal_coverage_mask(
		cosm,
		surface_owner,
		entry_point,
		entry_point + dir * max_len,
		only_born_before_step
	);

	const auto bucket_len = max_len / static_cast<real32>(DECAL_COVERAGE_BUCKETS);

	auto cursor = 0.f;
	auto power_left = power;
	auto gift_left = max_gift;

	/* Returns false once the bullet has run out of power. */
	auto walk = [&](const real32 stretch, const bool covered) {
		const auto power_rate = (covered ? 1.f - DECAL_FATIGUE_RELIEF : 1.f) / penetrability;
		const auto traversable = power_left / power_rate;

		if (traversable < stretch) {
			cursor += traversable;
			power_left = 0.f;
			return false;
		}

		power_left -= stretch * power_rate;
		cursor += stretch;

		if (covered) {
			gift_left -= stretch * DECAL_FATIGUE_RELIEF / penetrability;
		}

		return true;
	};

	for (std::size_t i = 0; i < DECAL_COVERAGE_BUCKETS; ++i) {
		auto left_in_bucket = bucket_len;

		if (::is_coverage_bucket_set(mask, i) && gift_left > 0.f) {
			/* The gift only lasts for so many px of the covered material. */
			const auto gift_lasts_for = gift_left * penetrability / DECAL_FATIGUE_RELIEF;
			const auto discounted = std::min(left_in_bucket, gift_lasts_for);

			if (!walk(discounted, true)) {
				break;
			}

			left_in_bucket -= discounted;
		}

		/* Past the gift budget, even a covered bucket costs the full price. */
		if (left_in_bucket > 0.f && !walk(left_in_bucket, false)) {
			break;
		}
	}

	return { std::min(cursor, max_len), max_gift - std::max(0.f, gift_left) };
}
