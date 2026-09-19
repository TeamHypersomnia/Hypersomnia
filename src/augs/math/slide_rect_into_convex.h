#pragma once
#include <cstddef>
#include <array>
#include <optional>
#include <algorithm>

#include "augs/math/vec2.h"
#include "augs/math/convex_polygon_helpers.h"

struct rect_slide_fit {
	vec2 center;
	real32 fitted_scale = 1.f;

	/*
		The direction the rectangle was actually slid along.
		On grazing impacts this is the entry edge's inward normal
		rather than the requested slide_dir.
	*/
	vec2 applied_slide_dir;
};

/*
	Slides a rotated rectangle into a convex polygon, entering at impact_point.

	The entry edge is looked up in the polygon itself rather than taken from the
	caller: a physics-reported impact point may lie slightly outside, and its
	normal may deviate from the polygon's own edge, both of which would
	collapse the scale bounds below.

	The rectangle is pushed in until its deepest corner rests on the entry edge,
	then downscaled until every corner is inside every edge. Grazing impacts
	(slide_dir nearly parallel to the entry edge) slide straight in instead.

	slide_dir must be normalized.

	Returns the final center and the applied scale [min_allowed_scale, 1] -
	never below min_allowed_scale, even at the cost of a slight overhang,
	so that a decal always spawns. std::nullopt only on a degenerate polygon.
*/
template <class VertsContainer>
std::optional<rect_slide_fit> slide_rect_into_convex(
	const VertsContainer& polygon,
	const vec2 impact_point,
	const vec2 slide_dir,
	const vec2 rect_size,
	const real32 rect_rotation_degs,
	const real32 min_allowed_scale
) {
	const auto num_verts = polygon.size();

	if (num_verts < 3) {
		return std::nullopt;
	}

	const auto centroid = augs::calc_centroid(polygon);

	auto entry_edge = std::size_t(0);
	const auto violation = augs::calc_max_edge_violation(polygon, centroid, impact_point, &entry_edge);

	const auto entry_normal = augs::outward_normal_of_edge(polygon, centroid, entry_edge);

	if (!(entry_normal.length_sq() > 0.f)) {
		return std::nullopt;
	}

	/* Snap a slightly-outside impact point onto its edge. */
	const auto entry_point =
		violation > 0.f ?
		impact_point - entry_normal * violation :
		impact_point
	;

	const auto applied_slide_dir =
		entry_normal.dot(slide_dir) < -0.2f ?
		slide_dir :
		-entry_normal
	;

	const auto approach = entry_normal.dot(applied_slide_dir);

	if (!(approach < -1e-4f)) {
		return std::nullopt;
	}

	const auto corners = augs::make_rotated_corners(rect_size, rect_rotation_degs);

	/* How deep the worst corner sticks out past the entry edge, per unit of scale. */
	auto deepest = 0.f;

	for (const auto& c : corners) {
		deepest = std::max(deepest, entry_normal.dot(c));
	}

	const auto slide_per_scale = applied_slide_dir * (deepest / -approach);

	/*
		Every corner's position is linear in the scale, so each
		(edge, corner) pair yields a closed-form upper bound on it.
	*/
	auto max_scale = 1.f;

	for (std::size_t i = 0; i < num_verts; ++i) {
		const auto edge_normal = augs::outward_normal_of_edge(polygon, centroid, i);

		/* Clamped to zero for robustness against the float error. */
		const auto entry_slack = std::min(0.f, edge_normal.dot(entry_point - vec2(polygon[i])));

		for (const auto& c : corners) {
			const auto growth = edge_normal.dot(slide_per_scale + c);

			if (growth > 1e-3f) {
				max_scale = std::min(max_scale, -entry_slack / growth);
			}
		}
	}

	max_scale = std::max(max_scale, min_allowed_scale);

	return rect_slide_fit {
		entry_point + slide_per_scale * max_scale,
		max_scale,
		applied_slide_dir
	};
}

/*
	Checks if a rotated rectangle centered at rect_center
	lies entirely inside a convex polygon, with eps_px of allowed overhang.
*/
template <class VertsContainer>
bool rect_inside_convex(
	const VertsContainer& polygon,
	const vec2 rect_center,
	const vec2 rect_size,
	const real32 rect_rotation_degs,
	const real32 eps_px
) {
	if (polygon.size() < 3) {
		return false;
	}

	const auto centroid = augs::calc_centroid(polygon);

	for (const auto& c : augs::make_rotated_corners(rect_size, rect_rotation_degs)) {
		if (augs::calc_max_edge_violation(polygon, centroid, rect_center + c) > eps_px) {
			return false;
		}
	}

	return true;
}
