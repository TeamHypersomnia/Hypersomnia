#pragma once
#include "rects.h"
#include <algorithm>

namespace augmentations {
	namespace rects {
		std::wostream& operator<<(std::wostream& ss, const point& val) {
			ss << '[';
			ss << val.x;
			ss << ',';
			ss << val.y;
			ss << ']';
			return ss;
		}
		
		std::wostream& operator<<(std::wostream& ss, const pointf& val) {
			ss << '[';
			ss << val.x;
			ss << ',';
			ss << val.y;
			ss << ']';
			return ss;
		}

		std::ostream& operator<<(std::ostream& ss, const point& val) {
			ss << '[';
			ss << val.x;
			ss << ',';
			ss << val.y;
			ss << ']';
			return ss;
		}

		std::ostream& operator<<(std::ostream& ss, const pointf& val) {
			ss << '[';
			ss << val.x;
			ss << ',';
			ss << val.y;
			ss << ']';
			return ss;
		}

		double deg2rad(double a) {
			return a*0.01745329251994329576923690768489;
		}

		point point::operator-() const {
			return operator*(-1);
		}

		point::point(int x, int y) : x(x), y(y) {}
		
		point::point(const pointf& p) : x(int(p.x)), y(int(p.y)) {} 
		point::point(const xywh& p) : x(int(p.x)), y(int(p.y)) {} 
		point::point(const ltrb& p) : x(int(p.l)), y(int(p.t)) {} 
		
		
		point point::operator-(const point& p) const {
			return point(x - p.x, y - p.y);
		}

		pointf point::operator-(const pointf& p) const {
			return pointf(x - p.x, y - p.y);
		}
		
		point point::operator+(const point& p) const {
			return point(x + p.x, y + p.y);
		}

		pointf point::operator+(const pointf& p) const {
			return pointf(x + p.x, y + p.y);
		}

		point point::operator*(const point& p) const {
			return point(x * p.x, y * p.y);
		}
		
		pointf point::operator*(const pointf& p) const {
			return pointf(x * p.x, y * p.y);
		}
		
		point& point::operator*=(int s) {
			x *= s;
			y *= s;
			return *this;
		}

		point& point::operator*=(float s) {
			x = int(float(x) * s);
			y = int(float(y) * s);
			return *this;
		}

		pointf point::operator*(float p) const {
			return pointf(x * p, y * p);
		}
		
		point& point::operator-=(const point& p) {
			x -= p.x;
			y -= p.y;
			return *this;
		}

		point& point::operator-=(const pointf& p) {
			x -= int(p.x);
			y -= int(p.y);
			return *this;
		}

		point& point::operator+=(const point& p) {
			x += p.x;
			y += p.y;
			return *this;
		}

		point& point::operator+=(const pointf& p) {
			x += int(p.x);
			y += int(p.y);
			return *this;
		}
		
		point& point::operator*=(const point& p) {
			x *= p.x;
			y *= p.y;
			return *this;
		}
		
		point& point::operator*=(const pointf& p) {
			x = int(float(x) * p.x);
			y = int(float(y) * p.y);
			return *this;
		}
		
		
		pointf::pointf(const point& p) : x(float(p.x)), y(float(p.y)) {}
		pointf::pointf(const xywh& p) : x(float(p.x)), y(float(p.y)) {}
		pointf::pointf(const ltrb& p) : x(float(p.l)), y(float(p.t)) {}
		
		pointf::pointf(float x, float y) : x(x), y(y) {}
			
		float pointf::length() const {
			return sqrt(x*x + y*y);
		}
		
		float pointf::get_radians() const {
			return atan2(y, x);
		}

		float pointf::get_degrees() const {
			return get_radians()*180.0/3.141592653589793238462;
		}

		void pointf::set_from_angle(float rotation) {
			x = sin(rotation);
			y = cos(rotation);
			normalize();
		}

		void pointf::normalize() {
			float len = 1.f/length();
			x *= len;
			y *= len;
		}
		
		pointf pointf::operator-(const point& p) const {
			return pointf(x - p.x, y - p.y);
		}

		pointf pointf::operator-(const pointf& p) const {
			return pointf(x - p.x, y - p.y);
		}

		pointf pointf::operator+(const point& p) const {
			return pointf(x + p.x, y + p.y);
		}

		pointf pointf::operator+(const pointf& p) const {
			return pointf(x + p.x, y + p.y);
		}
		
		pointf pointf::operator*(const point& p) const {
			return pointf(x * p.x, y * p.y);
		}
		
		pointf pointf::operator*(const pointf& p) const {
			return pointf(x * p.x, y * p.y);
		}
		
		pointf pointf::operator*(float p) const {
			return pointf(x * p, y * p);
		}
		
		pointf& pointf::operator-=(const point& p) {
			x -= p.x;
			y -= p.y;
			return *this;
		}

		pointf& pointf::operator-=(const pointf& p) {
			x -= p.x;
			y -= p.y;
			return *this;
		}

		pointf& pointf::operator+=(const point& p) {
			x += p.x;
			y += p.y;
			return *this;
		}

		pointf& pointf::operator+=(const pointf& p) {
			x += p.x;
			y += p.y;
			return *this;
		}
		
		pointf& pointf::operator*=(const point& p) {
			x *= p.x;
			y *= p.y;
			return *this;
		}
		
		pointf& pointf::operator*=(const pointf& p) {
			x *= p.x;
			y *= p.y;
			return *this;
		}

		wh::wh(const ltrb& rr) : w(rr.w()), h(rr.h()) {} 
		wh::wh(const xywh& rr) : w(rr.w), h(rr.h) {} 
		wh::wh(int w, int h) : w(w), h(h) {}
		
		wh::fit_status wh::fits(const wh& r) const {
			if(w == r.w && h == r.h) return fit_status::FITS_PERFECTLY;
			if(h == r.w && w == r.h) return fit_status::FITS_PERFECTLY_FLIPPED;
			if(w <= r.w && h <= r.h) return fit_status::FITS_INSIDE;
			if(h <= r.w && w <= r.h) return fit_status::FITS_INSIDE_FLIPPED;
			return fit_status::DOESNT_FIT;
		}
			
		wh wh::operator*(float s) const {
			return wh(int(w*s), int(h*s));
		}
			
		bool wh::operator==(const wh& r) const {
			return w == r.w && h == r.h;
		}
		
		ltrb::ltrb() : l(0), t(0), r(0), b(0) {}
		ltrb::ltrb(const wh& rr) : l(0), t(0), r(rr.w), b(rr.h) {}
		ltrb::ltrb(const xywh& rr) : l(rr.x), t(rr.y), r(rr.x+rr.w), b(rr.y+rr.h) {}
		ltrb::ltrb(int l, int t, int r, int b) : l(l), t(t), r(r), b(b) {}
		ltrb::ltrb(const point& p, const wh& s) : l(p.x), t(p.y), r(p.x + s.w), b(p.y + s.h) {}

		int ltrb::w() const {
			return r-l;
		}
		
		int ltrb::h() const {
			return b-t;
		}

		int ltrb::area() const {
			return w()*h();
		}

		int ltrb::perimeter() const {
			return 2*w() + 2*h();
		}

		int ltrb::max_side() const {
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
		
		bool ltrb::hover(const point& m) const {
			return m.x >= l && m.y >= t && m.x <= r && m.y <= b;
		}

		bool ltrb::hover(const ltrb& rc) const {
			return !(l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t);
		}
		
		bool ltrb::inside(const ltrb& rc) const {
			return l >= rc.l && r <= rc.r && t >= rc.t && b <= rc.b;
		}
		
		bool ltrb::stick_x(const ltrb& rc) {
			point offset(0, 0);
			if(l < rc.l) offset.x += rc.l - l; 
			if(r > rc.r) offset.x += rc.r - r; 
			operator+=(offset);

			return offset.x == 0;
		}

		bool ltrb::stick_y(const ltrb& rc) {
			point offset(0, 0);
			if(t < rc.t) offset.y += rc.t - t; 
			if(b > rc.b) offset.y += rc.b - b; 
			operator+=(offset);

			return offset.y == 0;
		}
		
		pointf ltrb::center() const {
			return pointf(l + w()/2.f, t + h()/2.f); 
		}

		void wh::stick_relative(const wh& content, pointf& scroll) const {
			scroll.x = std::min(scroll.x, float(content.w - w));
			scroll.x = std::max(scroll.x, 0.f);
			scroll.y = std::min(scroll.y, float(content.h - h));
			scroll.y = std::max(scroll.y, 0.f);
		}
		
		bool wh::inside(const wh& rc) const {
			return w <= rc.w && h <= rc.h;
		}

		bool wh::is_sticked(const wh& content, pointf& scroll) const {
			return scroll.x >= 0.f && scroll.x <= content.w - w && scroll.y >= 0 && scroll.y <= content.h - h; 
		}
		
		void ltrb::center_x(int c) {
			int _w = w();
			l = c - _w/2;
			r = l + _w;
		}

		void ltrb::center_y(int c) {
			int _h = h();
			t = c - _h/2;
			b = t + _h;
		}

		void ltrb::center(const point& c) {
			center_x(c.x);
			center_y(c.y);
		}
		
		void ltrb::x(int xx) {
			*this+=(point(xx-l, 0));
		}

		void ltrb::y(int yy) {
			*this+=(point(0, yy-t));
		}

		void ltrb::w(int ww) {
			r = l+ww;
		}
		
		void ltrb::h(int hh) {
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
		xywh::xywh(int x, int y, int w, int h) : x(x), y(y), wh(w, h) {}
		xywh::xywh(int x, int y, const wh& r) : x(x), y(y), wh(r) {}
		xywh::xywh(const point& p, const wh& r) : x(p.x), y(p.y), wh(r) {} 

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

		bool xywh::hover(const point& m) {
			return m.x >= x && m.y >= y && m.x <= r() && m.y <= b();
		}
		
		int xywh::r() const {
			return x+w;
		};
		
		int xywh::b() const {
			return y+h;
		}

		void xywh::r(int right) {
			w = right-x;
		}
		
		void xywh::b(int bottom) {
			h = bottom-y;
		}

		int wh::area() const {
			return w*h;
		}
		
		int wh::perimeter() const {
			return 2*w + 2*h; 
		}
		
		int wh::max_side() const {
			return std::max(w, h);
		}

		
		xywhf::xywhf(const ltrb& rr) : xywh(rr), flipped(false) {}
		xywhf::xywhf(const xywh& rr) : xywh(rr), flipped(false) {}
		xywhf::xywhf(const wh  & rr) : xywh(rr), flipped(false) {}
		xywhf::xywhf(int x, int y, int width, int height, bool flipped) : xywh(x, y, width, height), flipped(flipped) {}
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