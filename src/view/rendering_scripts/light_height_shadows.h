#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

#include "augs/math/vec2.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/detail/physics/physics_queries.h"
#include "view/rendering_scripts/shadow_casters.h"

/*
	At full smoothness, the ends of shadows of lights with a height fade over this fraction of the shadow's length.
*/

inline constexpr real32 LIGHT_SHADOW_CAP_SOFTNESS = 0.5f;

/*
	Number of columns across each side penumbra.
*/

inline constexpr int LIGHT_SHADOW_SIDE_COLUMNS = 6;

/*
	Largest angle between two columns of a shadow - 15 degrees keeps the chords within 1% of the reach's arc.
*/

inline constexpr real32 LIGHT_SHADOW_MAX_COLUMN_GAP = 15.0f * DEG_TO_RAD<real32>;

/*
	Shadows of a light standing at a height above the ground.

	The light is its whole reach, masked by the shadows drawn here into the alpha of the light texture
	with min blending, so that overlapping shadows combine correctly.
	The visibility polygon is not used at all.

	Every convex fixture throws its own shadow. Along any ray from the light,
	a fixture of height h shadows the ground from where the ray leaves the fixture (d_out)
	up to d_out * H / (H - h) - past that the light, at height H, sees the ground again over the fixture.
	Fixtures at least as tall as the light, the ones reaching the ceiling
	and the ones with no height at all - foreground walls - throw it to the end of the reach.

	The shadow is split into columns at the angles of the fixture's vertices, which keeps its boundaries exact:
	between two such angles the ray leaves the fixture through a single edge,
	and scaling that edge about the light keeps it straight.
	More columns go into the side penumbras, which fade the shadow in from its silhouette, like with lights without a height.
	Sides touched by other obstacles get none, so walls built of many tiles throw one seamless shadow.

	Every vertex carries texcoord x - from 0 at the silhouette to 1 past the side penumbra,
	and texcoord y - from 0 in the shadow to 1 past its faded end.
	light.fsh computes the mask from them per fragment.
*/

struct light_shadow_mask_input {
	const cosmos& cosm;
	const vec2 light_pos;
	const vec2 reach_size;
	const b2Filter filter;
	const entity_id subject;
	const real32 light_height;
	const real32 smoothness;
	const real32 max_penumbra_degrees;
	const rgba color;
	const real32 light_radius;
};

/*
	out receives the shadows of obstacles reaching the ceiling and the insides of all obstacles,
	low_out - the shadows of the lower obstacles.
	light_system.cpp keeps the hue of the light only through the latter, and only where the former don't block it,
	so that walls block the light honestly and the rooms behind them never take its hue.
*/

inline void build_light_shadow_masks(
	const light_shadow_mask_input in,
	augs::vertex_triangle_buffer& out,
	augs::vertex_triangle_buffer& low_out
) {
	out.clear();
	low_out.clear();

	auto is_low = false;

	const auto& cosm = in.cosm;
	const auto si = cosm.get_si();
	const auto& physics = cosm.get_solvable_inferred().physics;

	const auto L = in.light_pos;
	const auto H = in.light_height;
	const auto half_reach = in.reach_size / 2;
	const auto far_distance = half_reach.length() * 1.05f;
	const auto max_penumbra_radians = in.smoothness * in.max_penumbra_degrees * DEG_TO_RAD<real32>;

	thread_local std::vector<vec2> points;
	thread_local std::vector<real32> column_angles;

	auto push_vertex = [&](augs::vertex_triangle& tri, const int i, const vec2 pos, const vec2 texcoord) {
		tri.vertices[i].pos = pos;
		tri.vertices[i].texcoord = texcoord;
		tri.vertices[i].color = in.color;
	};

	auto push_quad = [&](
		const std::array<vec2, 4> positions,
		const std::array<vec2, 4> texcoords
	) {
		auto& target = is_low ? low_out : out;

		augs::vertex_triangle a;
		push_vertex(a, 0, positions[0], texcoords[0]);
		push_vertex(a, 1, positions[1], texcoords[1]);
		push_vertex(a, 2, positions[2], texcoords[2]);
		target.push_back(a);

		augs::vertex_triangle b;
		push_vertex(b, 0, positions[0], texcoords[0]);
		push_vertex(b, 1, positions[2], texcoords[2]);
		push_vertex(b, 2, positions[3], texcoords[3]);
		target.push_back(b);
	};

	const auto fully_dark = vec2(1.0f, 0.0f);

	physics.for_each_in_aabb(
		si,
		L - half_reach,
		L + half_reach,
		in.filter,
		[&](const b2Fixture& fix) {
			if (fix.IsSensor()) {
				return callback_result::CONTINUE;
			}

			const auto* const body = fix.GetBody();

			if (in.subject.is_set() && body->GetUserData() == FixtureUserdata(in.subject)) {
				return callback_result::CONTINUE;
			}

			::gather_fixture_world_points(points, fix, ::calc_physical_body_transform(*body, si), si, 16);

			const auto n = points.size();

			if (n < 3) {
				return callback_result::CONTINUE;
			}

			/*
				A fixture lying wholly beyond the reach of the light shadows nothing it lights.
			*/

			{
				auto lt = points[0];
				auto rb = points[0];

				for (const auto& p : points) {
					lt = vec2(std::min(lt.x, p.x), std::min(lt.y, p.y));
					rb = vec2(std::max(rb.x, p.x), std::max(rb.y, p.y));
				}

				const auto closest = vec2(std::clamp(L.x, lt.x, rb.x), std::clamp(L.y, lt.y, rb.y));

				if ((closest - L).length() > in.light_radius) {
					return callback_result::CONTINUE;
				}
			}

			auto center = vec2::zero;

			for (const auto& p : points) {
				center += p;
			}

			center /= static_cast<real32>(n);

			const auto light_inside = [&]() {
				for (std::size_t i = 0; i < n; ++i) {
					const auto a = points[i];
					const auto b = points[(i + 1) % n];

					const auto edge_side = (b - a).cross(L - a);
					const auto center_side = (b - a).cross(center - a);

					if (edge_side * center_side < 0.0f) {
						return false;
					}
				}

				return true;
			}();

			if (light_inside) {
				return callback_result::CONTINUE;
			}

			/*
				The fixture itself is always dark - obstacles aren't lit from inside.
			*/

			for (std::size_t i = 2; i < n; ++i) {
				augs::vertex_triangle tri;
				push_vertex(tri, 0, points[0], fully_dark);
				push_vertex(tri, 1, points[i - 1], fully_dark);
				push_vertex(tri, 2, points[i], fully_dark);
				out.push_back(tri);
			}

			const auto h = static_cast<real32>(::calc_fixture_shadow_height(cosm, fix));
			const bool reaches_ceiling = ::calc_fixture_reaches_ceiling(cosm, fix);
			const bool reaches_the_end = h <= 0.0f || h >= H || reaches_ceiling;

			is_low = h > 0.0f && !reaches_ceiling;

			/*
				Angles relative to the direction towards the center, which the convex fixture spans less than 180 degrees around.
			*/

			const auto reference = vec2(center - L);

			auto angle_of = [&](const vec2 p) {
				const auto d = p - L;
				return std::atan2(reference.cross(d), reference.dot(d));
			};

			auto min_angle = std::numeric_limits<real32>::max();
			auto max_angle = -std::numeric_limits<real32>::max();
			auto min_vertex = vec2::zero;
			auto max_vertex = vec2::zero;

			for (const auto& p : points) {
				const auto a = angle_of(p);

				if (a < min_angle) {
					min_angle = a;
					min_vertex = p;
				}

				if (a > max_angle) {
					max_angle = a;
					max_vertex = p;
				}
			}

			const auto span = max_angle - min_angle;

			if (span < 0.0001f) {
				return callback_result::CONTINUE;
			}

			const auto reference_dir = vec2(reference).normalize();

			/*
				A side of the silhouette touched by another obstacle - like the next tile of a wall laid out of many -
				is not an edge of the shadow at all, so it gets no penumbra.
				Otherwise the light would leak through the seams between the tiles.
			*/

			auto side_is_covered = [&](const vec2 silhouette_vertex, const real32 angle, const real32 outwards) {
				const auto distance = (silhouette_vertex - L).length();

				if (distance < 1.0f) {
					return false;
				}

				/*
					A point right past the corner, just outside the silhouette, must lie inside another obstacle.
					Obstacles merely further along the same direction don't count as touching.

					The neighbor may continue from the corner towards the light or away from it,
					with its edge slightly slanted, so points a little closer and a little further are probed too.
				*/

				const auto probe_margin = 2.0f;
				const auto probe_angle = angle + outwards * probe_margin / distance;
				const auto probe_dir = vec2(reference_dir).rotate_radians(probe_angle);

				const std::array<vec2, 3> probe_points = {
					L + probe_dir * (distance - probe_margin),
					L + probe_dir * distance,
					L + probe_dir * (distance + probe_margin)
				};

				const std::array<b2Vec2, 3> probes = {
					b2Vec2(si.get_meters(probe_points[0])),
					b2Vec2(si.get_meters(probe_points[1])),
					b2Vec2(si.get_meters(probe_points[2]))
				};

				auto probes_lt = probe_points[0];
				auto probes_rb = probe_points[0];

				for (const auto& p : probe_points) {
					probes_lt = vec2(std::min(probes_lt.x, p.x), std::min(probes_lt.y, p.y));
					probes_rb = vec2(std::max(probes_rb.x, p.x), std::max(probes_rb.y, p.y));
				}

				auto covered = false;

				physics.for_each_in_aabb(
					si,
					probes_lt - vec2::square(1.0f),
					probes_rb + vec2::square(1.0f),
					in.filter,
					[&](const b2Fixture& other) {
						if (&other == &fix || other.IsSensor()) {
							return callback_result::CONTINUE;
						}

						if (in.subject.is_set() && other.GetBody()->GetUserData() == FixtureUserdata(in.subject)) {
							return callback_result::CONTINUE;
						}

						for (const auto& probe : probes) {
							if (other.TestPoint(probe)) {
								covered = true;
								return callback_result::ABORT;
							}
						}

						return callback_result::CONTINUE;
					}
				);

				return covered;
			};

			const auto max_penumbra = std::min(max_penumbra_radians, span / 2);
			const auto min_side_penumbra = max_penumbra > 0.0f && !side_is_covered(min_vertex, min_angle, -1.0f) ? max_penumbra : 0.0f;
			const auto max_side_penumbra = max_penumbra > 0.0f && !side_is_covered(max_vertex, max_angle, 1.0f) ? max_penumbra : 0.0f;

			column_angles.clear();

			for (const auto& p : points) {
				column_angles.push_back(angle_of(p));
			}

			for (int k = 1; k <= LIGHT_SHADOW_SIDE_COLUMNS; ++k) {
				if (min_side_penumbra > 0.0f) {
					column_angles.push_back(min_angle + min_side_penumbra * k / LIGHT_SHADOW_SIDE_COLUMNS);
				}

				if (max_side_penumbra > 0.0f) {
					column_angles.push_back(max_angle - max_side_penumbra * k / LIGHT_SHADOW_SIDE_COLUMNS);
				}
			}

			std::sort(column_angles.begin(), column_angles.end());

			column_angles.erase(
				std::unique(
					column_angles.begin(),
					column_angles.end(),
					[](const real32 a, const real32 b) { return std::abs(a - b) < 0.00001f; }
				),
				column_angles.end()
			);

			/*
				Where a shadow is clamped to the reach, its end follows an arc around the light, not a straight edge.
				A straight quad between two columns far apart would cut a chord much closer to the light,
				leaving a lit wedge behind the obstacle - so no two columns may be too far apart.
			*/

			{
				const auto num_original = column_angles.size();

				for (std::size_t i = 1; i < num_original; ++i) {
					const auto from = column_angles[i - 1];
					const auto gap = column_angles[i] - from;
					const auto num_splits = static_cast<int>(std::ceil(gap / LIGHT_SHADOW_MAX_COLUMN_GAP));

					for (int k = 1; k < num_splits; ++k) {
						column_angles.push_back(from + gap * k / num_splits);
					}
				}

				std::sort(column_angles.begin(), column_angles.end());
			}

			struct column {
				vec2 shadow_start;
				vec2 core_end;
				vec2 faded_end;
				real32 side = 1.0f;
				bool has_faded_end = false;
			};

			auto make_column = [&](const real32 angle) -> std::optional<column> {
				const auto dir = vec2(reference_dir).rotate_radians(angle);

				/*
					Where the ray leaves the fixture.
					Rays at the silhouette only graze a vertex, hence the tolerance.
				*/

				auto exit_distance = -1.0f;

				for (std::size_t i = 0; i < n; ++i) {
					const auto a = points[i];
					const auto edge = points[(i + 1) % n] - a;
					const auto denom = dir.cross(edge);

					if (std::abs(denom) < 0.000001f) {
						continue;
					}

					const auto to_a = a - L;
					const auto t = to_a.cross(edge) / denom;
					const auto s = to_a.cross(dir) / denom;

					if (s >= -0.001f && s <= 1.001f && t > 0.0f) {
						exit_distance = std::max(exit_distance, t);
					}
				}

				/*
					A column must never be dropped - the quads on both of its sides would go with it,
					and with only a few columns per fixture that opens a wide hole in the shadow.
					A grazing ray that missed every edge takes the farthest projection of the vertices instead,
					and a fixture leaving past the reach is clamped to it.
				*/

				if (exit_distance <= 0.0f) {
					for (const auto& p : points) {
						exit_distance = std::max(exit_distance, (p - L).dot(dir));
					}
				}

				exit_distance = std::min(exit_distance, far_distance);

				if (exit_distance <= 0.0f) {
					return std::nullopt;
				}

				const auto end_distance = reaches_the_end ? far_distance : std::min(far_distance, exit_distance * H / (H - h));
				const auto cap = reaches_the_end ? 0.0f : in.smoothness * LIGHT_SHADOW_CAP_SOFTNESS * (end_distance - exit_distance);

				column c;
				c.shadow_start = L + dir * exit_distance;
				c.core_end = L + dir * std::max(exit_distance, end_distance - cap / 2);
				c.faded_end = L + dir * std::min(far_distance, end_distance + cap / 2);
				c.has_faded_end = cap > 0.0f;

				auto into_shadow = [](const real32 from_side, const real32 side_penumbra) {
					return side_penumbra > 0.0f ? std::clamp(from_side / side_penumbra, 0.0f, 1.0f) : 1.0f;
				};

				c.side = std::min(
					into_shadow(angle - min_angle, min_side_penumbra),
					into_shadow(max_angle - angle, max_side_penumbra)
				);

				return c;
			};

			std::optional<column> previous;

			for (const auto angle : column_angles) {
				const auto current = make_column(angle);

				if (previous.has_value() && current.has_value()) {
					const auto& p = *previous;
					const auto& c = *current;

					push_quad(
						{ p.shadow_start, p.core_end, c.core_end, c.shadow_start },
						{ vec2(p.side, 0.0f), vec2(p.side, 0.0f), vec2(c.side, 0.0f), vec2(c.side, 0.0f) }
					);

					if (p.has_faded_end || c.has_faded_end) {
						push_quad(
							{ p.core_end, p.faded_end, c.faded_end, c.core_end },
							{ vec2(p.side, 0.0f), vec2(p.side, 1.0f), vec2(c.side, 1.0f), vec2(c.side, 0.0f) }
						);
					}
				}

				previous = current;
			}

			return callback_result::CONTINUE;
		}
	);
}
