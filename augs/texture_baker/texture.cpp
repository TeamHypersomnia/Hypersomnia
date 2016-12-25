#include "texture.h"
#include "image.h"

namespace augs {
	void texture::set_uv_unit(double u, double v) {
		x = float(double(rect.x) * u);
		y = float(double(rect.y) * v);
		w = float(double(rect.w) * u);
		h = float(double(rect.h) * v);
	}

	texture::texture(const image& img) : rect(rect_xywhf(0, 0, img.get_size().x, img.get_size().y)), ltoa(false) {}

	void texture::set(const image& img) {
		rect = rect_xywhf(0, 0, img.get_size().x, img.get_size().y);
	}

	void texture::luminosity_to_alpha(bool flag) {
		ltoa = flag;
	}

	rects::xywhf<int> texture::get_rect() const {
		return rects::xywhf<int>(rect.x, rect.y, rect.w, rect.h, rect.flipped);
	}

	vec2i texture::get_size() const {
		return !rect.flipped ? vec2i(rect.w, rect.h) : vec2i(rect.h, rect.w);
	}

	void texture::get_uv(float u, float v, float& u_out, float& v_out) const {
		if (!rect.flipped) {
			u_out = x + w * u;
			v_out = y + h * v;
		}
		else {
			u_out = x + w * v;
			v_out = y + h * u;
		}
	}

	void texture::get_uv(vec2& texture_space) const {
		auto temp = texture_space;
		get_uv(temp.x, temp.y, texture_space.x, texture_space.y);
	}

	float texture::get_u(int vertex_num_from_cw_rect) const {
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

	float texture::get_v(int vertex_num_from_cw_rect) const {
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

	void texture::translate_uv(vec2 uv) {
		uv *= vec2(w / rect.w, h / rect.h);
		x += uv.x;
		y += uv.y;
	}

	void texture::scale_uv(float u_scalar, float v_scalar) {
		w *= u_scalar;
		h *= v_scalar;
	}

	void texture::get_uv(const rects::texture<float> &uv, rects::texture<float>& out) const {
		get_uv(uv.u1, uv.v1, out.u1, out.v1);
		get_uv(uv.u2, uv.v2, out.u2, out.v2);
	}

	float texture::get_u_unit() const {
		return rect.flipped ? h : w;
	}

	float texture::get_v_unit() const {
		return rect.flipped ? w : h;
	}
}