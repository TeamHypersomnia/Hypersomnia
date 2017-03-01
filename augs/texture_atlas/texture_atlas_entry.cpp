#include "texture_atlas_entry.h"
#include "augs/image/image.h"

namespace augs {
	void texture_atlas_entry::set_uv_unit(double u, double v) {
		x = float(double(rect.x) * u);
		y = float(double(rect.y) * v);
		w = float(double(rect.w) * u);
		h = float(double(rect.h) * v);
	}

	texture_atlas_entry::texture_atlas_entry(const image& img) : rect(rect_xywhf(0, 0, img.get_size().x, img.get_size().y)), ltoa(false) {}

	void texture_atlas_entry::set(const image& img) {
		rect = rect_xywhf(0, 0, img.get_size().x, img.get_size().y);
	}

	rect_xywhf texture_atlas_entry::get_rect() const {
		return rect;
	}

	vec2i texture_atlas_entry::get_size() const {
		return !rect.flipped ? vec2i(rect.w, rect.h) : vec2i(rect.h, rect.w);
	}

	void texture_atlas_entry::get_atlas_space_uv(float u, float v, float& u_out, float& v_out) const {
		if (!rect.flipped) {
			u_out = x + w * u;
			v_out = y + h * v;
		}
		else {
			u_out = x + w * v;
			v_out = y + h * u;
		}
	}

	void texture_atlas_entry::get_atlas_space_uv(vec2& texture_space) const {
		auto temp = texture_space;
		get_atlas_space_uv(temp.x, temp.y, texture_space.x, texture_space.y);
	}

	float texture_atlas_entry::get_u(int vertex_num_from_cw_rect) const {
		vertex_num_from_cw_rect %= 4;
		float u = 0.f;

		switch (vertex_num_from_cw_rect) {
		case 0: u = 0.f; break;
		case 1: u = rect.flipped ? 0.f : 1.f; break;
		case 2: u = 1.f; break;
		case 3: u = rect.flipped ? 1.f : 0.f; break;
		default: break;
		}

		return x + w * u;
	}

	float texture_atlas_entry::get_v(int vertex_num_from_cw_rect) const {
		vertex_num_from_cw_rect %= 4;
		float v = 0.f;

		switch (vertex_num_from_cw_rect) {
		case 0: v = 0.f; break;
		case 1: v = rect.flipped ? 1.f : 0.f; break;
		case 2: v = 1.f; break;
		case 3: v = rect.flipped ? 0.f : 1.f; break;
		default: break;
		}

		return y + h * v;
	}

	void texture_atlas_entry::get_atlas_space_uv(const rects::texture<float> &uv, rects::texture<float>& out) const {
		get_atlas_space_uv(uv.u1, uv.v1, out.u1, out.v1);
		get_atlas_space_uv(uv.u2, uv.v2, out.u2, out.v2);
	}

	float texture_atlas_entry::get_u_unit() const {
		return rect.flipped ? h : w;
	}

	float texture_atlas_entry::get_v_unit() const {
		return rect.flipped ? w : h;
	}
}