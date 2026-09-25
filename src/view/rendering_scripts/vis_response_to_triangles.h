#pragma once
#include <cstddef>
#include <array>
#include "game/inferred_caches/physics_world_cache.h"

inline void vis_response_to_triangles(
	const visibility_response& response, 
	augs::vertex_triangle_buffer& triangles, 
	const rgba col,
	const vec2 eye_pos
) {
	const auto triangles_n = response.get_num_triangles();
	triangles.resize(triangles_n);

	for (std::size_t t = 0; t < triangles_n; ++t) {
		const auto world_light_tri = response.get_world_triangle(t, eye_pos);

		auto& renderable_light_tri = triangles[t];

		for (int i = 0; i < 3; ++i) {
			renderable_light_tri.vertices[i].pos = world_light_tri[i];
			renderable_light_tri.vertices[i].color = col;
			renderable_light_tri.vertices[i].texcoord = vec2::zero;
		}
	}
};

/*
	At full smoothness, the penumbra spans this angle around the light.
*/

inline constexpr real32 MAX_LIGHT_PENUMBRA_DEGREES = 12.0f;

/*
	Number of strips approximating the arc of the penumbra.
*/

inline constexpr int LIGHT_PENUMBRA_STRIPS = 6;

/*
	Softens the shadow edges of a light.

	Every discontinuity is a hard shadow edge running along a ray,
	from the occluder's corner to where the ray lands behind it.
	The penumbra is that edge swept around the light, into the shadow.
	Each swept ray starts behind the occluder, found by casting from the far end back towards the light,
	so the penumbra never lights the occluder itself and stays put when the silhouette jumps
	between two corners on the same ray, e.g. when a face of a box turns edge-on to the light.

	Its vertices carry the position across the penumbra in texcoord.x and a marker in texcoord.y,
	and light.fsh fades both the color and the alpha along a smoothstep per fragment,
	so that neither edge of the penumbra shows up as a line.

	The side of the shadow comes from the discontinuity's winding.
*/

inline void append_light_penumbras(
	const visibility_response& response, 
	augs::vertex_triangle_buffer& triangles, 
	const rgba col,
	const vec2 eye_pos,
	const real32 smoothness,
	const physics_world_cache& physics,
	const si_scaling si,
	const b2Filter filter
) {
	triangles.clear();

	if (smoothness <= 0.0f) {
		return;
	}

	using discontinuity_type = messages::visibility_information_response::discontinuity;

	const auto max_degrees = smoothness * MAX_LIGHT_PENUMBRA_DEGREES;

	auto push_triangle = [&](const std::array<vec2, 3> positions, const std::array<real32, 3> ts) {
		augs::vertex_triangle tri;

		for (std::size_t i = 0; i < 3; ++i) {
			tri.vertices[i].pos = positions[i];
			tri.vertices[i].color = col;
			tri.vertices[i].texcoord = vec2(ts[i], 1.0f);
		}

		triangles.push_back(tri);
	};

	/*
		A corner is sometimes reported twice with slightly different landings.
	*/

	const auto duplicate_epsilon = 1.0f;

	for (std::size_t d = 0; d < response.discontinuities.size(); ++d) {
		const auto& disc = response.discontinuities[d];
		const auto corner = disc.points.first;
		const auto landing = disc.points.second;

		if ((landing - corner).length() < 1.0f) {
			continue;
		}

		const bool is_duplicate = [&]() {
			for (std::size_t earlier = 0; earlier < d; ++earlier) {
				if (response.discontinuities[earlier].points.first.compare(corner, duplicate_epsilon)) {
					return true;
				}
			}

			return false;
		}();

		if (is_duplicate) {
			continue;
		}

		/*
			Rays are sorted by increasing angle and a positive rotation increases the angle.
			A discontinuity winding to the left has the occluder, and so the shadow, on its greater angle side.
		*/

		const auto degrees_into_shadow = disc.winding == discontinuity_type::LEFT ? max_degrees : -max_degrees;

		auto swept = [&](const vec2 p, const real32 t) {
			return eye_pos + vec2(p - eye_pos).rotate(degrees_into_shadow * t);
		};

		std::array<vec2, LIGHT_PENUMBRA_STRIPS + 1> starts;
		std::array<vec2, LIGHT_PENUMBRA_STRIPS + 1> ends;

		for (int i = 0; i <= LIGHT_PENUMBRA_STRIPS; ++i) {
			const auto t = static_cast<real32>(i) / LIGHT_PENUMBRA_STRIPS;
			const auto end = swept(landing, t);

			ends[i] = end;

			if (i == 0) {
				/*
					The unswept ray only grazes the corner.
				*/

				starts[i] = corner;
				continue;
			}

			/*
				Cast from slightly off the far end, so as not to hit the surface it lies on.
			*/

			const auto cast_from = end + vec2(eye_pos - end).normalize() * 2.0f;
			const auto behind_occluder = physics.ray_cast_px(si, cast_from, eye_pos, filter);

			starts[i] = behind_occluder.hit ? behind_occluder.intersection : swept(corner, t);
		}

		for (int i = 0; i < LIGHT_PENUMBRA_STRIPS; ++i) {
			const auto t0 = static_cast<real32>(i) / LIGHT_PENUMBRA_STRIPS;
			const auto t1 = static_cast<real32>(i + 1) / LIGHT_PENUMBRA_STRIPS;

			push_triangle({ starts[i], ends[i], ends[i + 1] }, { t0, t0, t1 });
			push_triangle({ starts[i], ends[i + 1], starts[i + 1] }, { t0, t1, t1 });
		}
	}
}
