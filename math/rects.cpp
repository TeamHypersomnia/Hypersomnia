#pragma once
#include "rects.h"
#include "vec2d.h"
#include <algorithm>

namespace augmentations {
	namespace rects {
		
		wh::wh(const ltrb& rr) : w(rr.w()), h(rr.h()) {} 
		wh::wh(const xywh& rr) : w(rr.w), h(rr.h) {} 
		wh::wh(float w, float h) : w(w), h(h) {}
		
		wh::fit_status wh::fits(const wh& r) const {
			if(w == r.w && h == r.h) return fit_status::FITS_PERFECTLY;
			if(h == r.w && w == r.h) return fit_status::FITS_PERFECTLY_FLIPPED;
			if(w <= r.w && h <= r.h) return fit_status::FITS_INSIDE;
			if(h <= r.w && w <= r.h) return fit_status::FITS_INSIDE_FLIPPED;
			return fit_status::DOESNT_FIT;
		}
			
		wh wh::operator*(float s) const {
			return wh(float(w*s), float(h*s));
		}

		bool wh::operator==(const wh& r) const {
			return w == r.w && h == r.h;
		}
		
		ltrb::ltrb() : l(0), t(0), r(0), b(0) {}
		ltrb::ltrb(const wh& rr) : l(0), t(0), r(rr.w), b(rr.h) {}
		ltrb::ltrb(const xywh& rr) : l(rr.x), t(rr.y), r(rr.x+rr.w), b(rr.y+rr.h) {}
		ltrb::ltrb(float l, float t, float r, float b) : l(l), t(t), r(r), b(b) {}
		ltrb::ltrb(const vec2<float>& p, const wh& s) : l(p.x), t(p.y), r(p.x + s.w), b(p.y + s.h) {}

		float ltrb::w() const {
			return r-l;
		}
		
		float ltrb::h() const {
			return b-t;
		}

		float ltrb::area() const {
			return w()*h();
		}

		float ltrb::perimeter() const {
			return 2*w() + 2*h();
		}

		float ltrb::max_side() const {
			return std::max(w(), h());
		}
		
		void ltrb::contain(const ltrb& rc) {
			l = std::min(l, rc.l);
			t = std::min(t, rc.t);
			contain_positive(rc);
		}

		void ltrb::contain_positive(const ltrb& rc) {
			r = std::max(r, rc.r);
			b = std::max(b, rc.b);
		}

		bool ltrb::clip(const ltrb& rc) {
			if(l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t) {
				*this = ltrb();
				return false;
			}
			*this = ltrb(std::max(l, rc.l),
				std::max(t, rc.t),
				std::min(r, rc.r),
				std::min(b, rc.b));
			return true;
		}

		bool ltrb::hover(const ltrb& rc) const {
			return !(l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t);
		}

		bool ltrb::hover(const xywh& rc) const {
			return hover(ltrb(rc));
		}
		
		bool ltrb::inside(const ltrb& rc) const {
			return l >= rc.l && r <= rc.r && t >= rc.t && b <= rc.b;
		}
		
		bool ltrb::stick_x(const ltrb& rc) {
			vec2<float> offset(0, 0);
			if(l < rc.l) offset.x += rc.l - l; 
			if(r > rc.r) offset.x += rc.r - r; 
			operator+=(offset);

			return offset.x == 0;
		}

		bool ltrb::stick_y(const ltrb& rc) {
			vec2<float> offset(0, 0);
			if(t < rc.t) offset.y += rc.t - t; 
			if(b > rc.b) offset.y += rc.b - b; 
			operator+=(offset);

			return offset.y == 0;
		}
		
		vec2<float> ltrb::center() const {
			return vec2<float>(l + w()/2.f, t + h()/2.f); 
		}

		void wh::stick_relative(const wh& content, vec2<float>& scroll) const {
			scroll.x = std::min(scroll.x, float(content.w - w));
			scroll.x = std::max(scroll.x, 0.f);
			scroll.y = std::min(scroll.y, float(content.h - h));
			scroll.y = std::max(scroll.y, 0.f);
		}
		
		bool wh::inside(const wh& rc) const {
			return w <= rc.w && h <= rc.h;
		}

		bool wh::is_sticked(const wh& content, vec2<float>& scroll) const {
			return scroll.x >= 0.f && scroll.x <= content.w - w && scroll.y >= 0 && scroll.y <= content.h - h; 
		}
		
		void ltrb::center_x(float c) {
			float _w = w();
			l = c - _w/2;
			r = l + _w;
		}

		void ltrb::center_y(float c) {
			float _h = h();
			t = c - _h/2;
			b = t + _h;
		}

		void ltrb::center(const vec2<float>& c) {
			center_x(c.x);
			center_y(c.y);
		}

		void ltrb::x(float xx) {
			*this+=(vec2<float>(xx-l, 0));
		}

		void ltrb::y(float yy) {
			*this+=(vec2<float>(0, yy-t));
		}

		void ltrb::w(float ww) {
			r = l+ww;
		}
		
		void ltrb::h(float hh) {
			b = t+hh;
		}

		bool ltrb::good() const {
			return w() > 0 && h() > 0;
		}
		
		bool wh::  good() const {
			return w   > 0 && h   > 0;
		}

		xywh::xywh() : x(0), y(0) {}
		xywh::xywh(const wh& rr) : x(0), y(0), wh(rr) {}
		xywh::xywh(const ltrb& rc) : x(rc.l), y(rc.t) { b(rc.b); r(rc.r); }
		xywh::xywh(float x, float y, float w, float h) : x(x), y(y), wh(w, h) {}
		xywh::xywh(float x, float y, const wh& r) : x(x), y(y), wh(r) {}
		xywh::xywh(const vec2<float>& p, const wh& r) : x(p.x), y(p.y), wh(r) {} 

		bool xywh::clip(const xywh& rc) {
			if(x >= rc.r() || y >= rc.b() || r() <= rc.x || b() <= rc.y) {
				*this = xywh();
				return false;
			}
			*this = ltrb(std::max(x, rc.x),
				std::max(y, rc.y),
				std::min(r(), rc.r()),
				std::min(b(), rc.b()));
			return true;
		}

		bool xywh::hover(const vec2<float>& m) {
			return m.x >= x && m.y >= y && m.x <= r() && m.y <= b();
		}

		bool xywh::hover(const ltrb& rc) {
			return rc.hover(*this);
		}

		bool xywh::hover(const xywh& rc) {
			return ltrb(rc).hover(*this);
		}
		
		float xywh::r() const {
			return x+w;
		};
		
		float xywh::b() const {
			return y+h;
		}

		void xywh::r(float right) {
			w = right-x;
		}
		
		void xywh::b(float bottom) {
			h = bottom-y;
		}

		float wh::area() const {
			return w*h;
		}
		
		float wh::perimeter() const {
			return 2*w + 2*h; 
		}
		
		float wh::max_side() const {
			return std::max(w, h);
		}

		
		xywhf::xywhf(const ltrb& rr) : xywh(rr), flipped(false) {}
		xywhf::xywhf(const xywh& rr) : xywh(rr), flipped(false) {}
		xywhf::xywhf(const wh  & rr) : xywh(rr), flipped(false) {}
		xywhf::xywhf(float x, float y, float width, float height, bool flipped) : xywh(x, y, width, height), flipped(flipped) {}
		xywhf::xywhf() : flipped(false) {}

		void xywhf::flip() { 
			flipped = !flipped;
			std::swap(w, h);
		}
		
		xywh xywhf::rc() const {
			return xywh(x, y, flipped ? h : w, flipped ? w : h);
		}
		
		bool xywh::operator==(const xywh& r) const {
			return x == r.x && y == r.y && wh::operator==(r);
		}

		texture::texture(float u1, float v1, float u2, float v2) : u1(u1), v1(v1), u2(u2), v2(v2) {}

	}
}