#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "3rdparty/rectpack2D/src/pack.h"

namespace augs {
	class image;

	class texture {
		friend class atlas;
		rect_xywhf rect;
		float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
		bool ltoa = false;

		void set_uv_unit(double, double);
	public:

		texture() = default;
		texture(const image& img);
		void set(const image& img);
		void luminosity_to_alpha(bool);

		rects::xywhf<int> get_rect() const;
		vec2i get_size() const;

		void get_atlas_space_uv(const rects::texture<float>& uv, rects::texture<float>& out) const;
		void get_atlas_space_uv(float u, float v, float& u_out, float& v_out) const;
		void get_atlas_space_uv(vec2& texture_space) const;

		/* gets u coordinate from a standard rectangular quad with origin coordinates 0.0, 0.0, 1.0, 1.0 */
		float get_u(int vertex_num_from_cw_rect) const;
		/* gets v coordinate from a standard rectangular quad with origin coordinates 0.0, 0.0, 1.0, 1.0 */
		float get_v(int vertex_num_from_cw_rect) const;

		float get_u_unit() const, get_v_unit() const;
	};
}