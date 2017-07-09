#pragma once
#include <vector>
#include <iosfwd>
#include <algorithm>
#include "declare_math.h"
// trzeba usprawnic rect2D - rozszerzyc max_size na wh bo z samego max_s: wielkie prostokaty rozpychaja kwadrat a male wykorzystuja miejsce na dole

struct b2Vec2;

enum rectangle_sticking {
	LEFT,
	RIGHT,
	TOP,
	BOTTOM
};
template <class T> struct ltrbt;
template <class T> struct xywht;

template <class T>
struct ltrbt {
	// GEN INTROSPECTOR struct ltrbt class T
	T l;
	T t;
	T r;
	T b;
	// END GEN INTROSPECTOR

	ltrbt() : l(static_cast<T>(0)), t(static_cast<T>(0)), r(static_cast<T>(0)), b(static_cast<T>(0)) {}

	template <class B>
	ltrbt(const xywht<B>& rr) : l(static_cast<T>(rr.x)), t(static_cast<T>(rr.y)), r(static_cast<T>(rr.x + rr.w)), b(static_cast<T>(rr.y + rr.h)) {}

	template <class B>
	ltrbt(const ltrbt<B>& rr) : l(static_cast<T>(rr.l)), t(static_cast<T>(rr.t)), r(static_cast<T>(rr.r)), b(static_cast<T>(rr.b)) {}

	ltrbt(const T l, const T t, const T r, const T b) : l(l), t(t), r(r), b(b) {}
	ltrbt(const vec2t<T> pos, const vec2t<T> size) : l(pos.x), t(pos.y), r(pos.x + size.x), b(pos.y + size.y) {}

	void x(const T xx) {
		*this += (vec2t<T>(xx - l, 0));
	}

	void y(const T yy) {
		*this += (vec2t<T>(0, yy - t));
	}

	void w(const T ww) {
		r = l + ww;
	}

	void h(const T hh) {
		b = t + hh;
	}

	void set(const T _l, const T _t, const T _r, const T _b) {
		*this = ltrbt(_l, _t, _r, _b);
	}

	auto& set_position(const vec2t<T> v) {
		const auto old_w = w();
		const auto old_h = h();

		l = v.x;
		t = v.y;
		w(old_w);
		h(old_h);

		return *this;
	}

	auto& set_size(const vec2t<T> v) {
		w(v.x);
		h(v.y);

		return *this;
	}

	auto& set_size(const T x, const T y) {
		w(x);
		h(y);

		return *this;
	}

	auto& contain(const ltrbt rc) {
		l = std::min(l, rc.l);
		t = std::min(t, rc.t);
		contain_positive(rc);

		return *this;
	}

	auto& contain_positive(const ltrbt rc) {
		r = std::max(r, rc.r);
		b = std::max(b, rc.b);

		return *this;
	}

	bool clip_by(const ltrbt rc) {
		if (l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t) {
			*this = ltrbt();
			return false;
		}
		*this = ltrbt(std::max(l, rc.l),
			std::max(t, rc.t),
			std::min(r, rc.r),
			std::min(b, rc.b));
		return true;
	}

	ltrbt& expand_from_center(const vec2 amount) {
		l -= amount.x;
		t -= amount.y;
		r += amount.x;
		b += amount.y;

		return *this;
	}

	ltrbt& snap_to_bounds(const ltrbt rc) {
		vec2t<T> offset(0, 0);

		if (l < rc.l) offset.x += rc.l - l;
		if (r > rc.r) offset.x += rc.r - r;
		if (t < rc.t) offset.y += rc.t - t;
		if (b > rc.b) offset.y += rc.b - b;

		operator+=(offset);

		return *this;
	}

	ltrbt& place_in_center_of(ltrbt bigger) {
		bigger.l += static_cast<T>(bigger.w() / 2.f - w() / 2.f);
		bigger.t += static_cast<T>(bigger.h() / 2.f - h() / 2.f);
		bigger.w(w());
		bigger.h(h());

		*this = bigger;
		return *this;
	}

	ltrbt& center_x(const T c) {
		const T _w = w();
		l = c - _w / 2;
		r = l + _w;

		return *this;
	}

	ltrbt& center_y(const T c) {
		const T _h = h();
		t = c - _h / 2;
		b = t + _h;

		return *this;
	}

	ltrbt& center(const vec2t<T> c) {
		center_x(c.x);
		center_y(c.y);

		return *this;
	}

	T w() const {
		return r - l;
	}

	T h() const {
		return b - t;
	}

	vec2t<T> left_top() const {
		return vec2t<T>(l, t);
	}

	vec2t<T> right_bottom() const {
		return vec2t<T>(r, b);
	}

	vec2t<T> right_top() const {
		return vec2t<T>(r, t);
	}

	vec2t<T> left_bottom() const {
		return vec2t<T>(l, b);
	}

	T perimeter() const {
		return 2 * w() + 2 * h();
	}

	T max_side() const {
		return std::max(w(), h());
	}

	bool hover(const ltrbt& rc) const {
		return !(l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t);
	}

	bool hover(const xywht<T>& rc) const {
		return hover(ltrbt(rc));
	}

	bool inside(const ltrbt& rc) const {
		return l >= rc.l && r <= rc.r && t >= rc.t && b <= rc.b;
	}

	vec2t<T> get_sticking_offset(const rectangle_sticking mode) const {
		vec2t<T> res;
		switch (mode) {
		case rectangle_sticking::LEFT: res = vec2t<T>(-r, 0);	break;
		case rectangle_sticking::RIGHT: res = vec2t<T>(-l, 0);	break;
		case rectangle_sticking::TOP: res = vec2t<T>(0, -b);	break;
		case rectangle_sticking::BOTTOM: res = vec2t<T>(0, -t);	break;
		default: res = vec2t<T>(0, 0); break;
		}

		return res;
	}

	vec2t<T> center() const {
		return vec2t<T>(l + w() / 2.f, t + h() / 2.f);
	}

	template <typename S>
	bool hover(const vec2t<S>& m) const {
		return m.x >= l && m.y >= t && m.x <= r && m.y <= b;
	}

	template <class S>
	std::array<vec2t<S>, 4> get_vertices() const {
		return{ vec2t<S>(l, t),
		vec2t<S>(r, t),
		vec2t<S>(r, b),
		vec2t<S>(l, b) };
	}

	template <typename type>
	void snap_point(vec2t<type>& v) const {
		if (v.x < l) v.x = static_cast<type>(l);
		if (v.y < t) v.y = static_cast<type>(t);
		if (v.x > r) v.x = static_cast<type>(r);
		if (v.y > b) v.y = static_cast<type>(b);
	}

	T area() const {
		return w()*h();
	}

	T diagonal() const {
		return sqrt(w()*w() + h()*h());
	}

	bool good() const {
		return w() > 0 && h() > 0;
	}

	vec2t<T> get_position() const {
		return vec2t<T>(l, t);
	}

	vec2t<T> get_size() const {
		return vec2t<T>(w(), h());
	}

	template <class P>
	ltrbt& operator+=(const P& p) {
		l += T(p.x);
		t += T(p.y);
		r += T(p.x);
		b += T(p.y);
		return *this;
	}

	template <class P>
	ltrbt operator-(const P& p) const {
		return ltrbt(l - T(p.x), t - T(p.y), r - T(p.x), b - T(p.y));
	}

	template <class P>
	ltrbt operator+(const P& p) const {
		return ltrbt(l + T(p.x), t + T(p.y), r + T(p.x), b + T(p.y));
	}

	ltrbt& scale(const T s) {
		l *= s;
		t *= s;
		r *= s;
		b *= s;

		return *this;
	}

	bool operator==(const ltrbt& a) {
		return l == a.l && r == a.r && t == a.t && b == a.b;
	}
};

template <class T>
struct xywht {
	// GEN INTROSPECTOR struct xywht class T
	T x;
	T y;
	T w;
	T h;
	// END GEN INTROSPECTOR

	xywht() : x(static_cast<T>(0)), y(static_cast<T>(0)), w(static_cast<T>(0)), h(static_cast<T>(0)) {}
	xywht(const ltrbt<T> rc) : x(rc.l), y(rc.t) { b(rc.b); r(rc.r); }
	xywht(const T x, const T y, const T w, const T h) : x(x), y(y), w(w), h(h) {}
	xywht(const T x, const T y, const vec2t<T>& s) : x(x), y(y), w(s.x), h(s.y) {}
	xywht(const vec2t<T>& p, const vec2t<T>& s) : x(p.x), y(p.y), w(s.x), h(s.y) {}

	void set(const T _x, const T _y, const T _w, const T _h) {
		*this = xywht(_x, _y, _w, _h);
	}

	auto& set_position(const vec2t<T> v) {
		x = v.x;
		y = v.y;

		return *this;
	}

	vec2t<T> get_position() const {
		return { x, y } 
	}

	bool clip(const xywht& rc) {
		if (x >= rc.r() || y >= rc.b() || r() <= rc.x || b() <= rc.y) {
			*this = xywht();
			return false;
		}
		*this = ltrbt<T>(std::max(x, rc.x),
			std::max(y, rc.y),
			std::min(r(), rc.r()),
			std::min(b(), rc.b()));
		return true;
	}

	void r(const T right) {
		this->w = right - x;
	}

	void b(const T bottom) {
		this->h = bottom - y;
	}

	xywht& expand_from_center(const vec2 amount) {
		x -= amount.x;
		y -= amount.y;
		w += amount.x;
		h += amount.y;

		return *this;
	}

	T r() const {
		return x + this->w;
	};

	T b() const {
		return y + this->h;
	}

	vec2t<T> center() const {
		return{ x + this->w / 2, y + this->h / 2 };
	}

	auto& set_size(const vec2t<T> v) {
		w = v.x;
		h = v.y;

		return *this;
	}

	vec2t<T> get_size() const {
		return{ w, h };
	}

	bool hover(const vec2t<T> m) const {
		return m.x >= x && m.y >= y && m.x <= r() && m.y <= b();
	}

	bool hover(const ltrbt<T> rc) const {
		return rc.hover(*this);
	}

	bool hover(const xywht rc) const {
		return ltrbt<T>(rc).hover(*this);
	}

	bool good() const {
		return w > 0 && h > 0;
	}

	T area() const {
		return w*h;
	}

	T perimeter() const {
		return 2 * w + 2 * h;
	}

	T max_side() const {
		return std::max(w, h);
	}

	bool operator==(const xywht r) const {
		return x == r.x && y == r.y && w == r.w && h == r.h;
	}

	template <class P>
	xywht& operator+=(const P& p) {
		x += T(p.x);
		y += T(p.y);
		return *this;
	}

	template <class P>
	xywht operator-(const P& p) const {
		return xywht(x - T(p.x), y - T(p.y), this->w, this->h);
	}

	template <class P>
	xywht operator+(const P& p) const {
		return xywht(x + T(p.x), y + T(p.y), this->w, this->h);
	}
};