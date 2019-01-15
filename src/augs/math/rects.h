#pragma once
#include <array>
#include <vector>
#include <iosfwd>
#include <algorithm>
#include <cmath>
#include "declare_math.h"
#include "augs/math/si_scaling.h"
#include "augs/math/repro_math.h"
#include "augs/string/typesafe_sprintf.h"

enum rectangle_sticking {
	LEFT,
	RIGHT,
	TOP,
	BOTTOM
};

template <class T> struct basic_ltrb;
template <class T> struct basic_xywh;

template <class T>
struct basic_ltrb {
	using vec2_type = basic_vec2<T>;

	// GEN INTROSPECTOR struct basic_ltrb class T
	T l;
	T t;
	T r;
	T b;
	// END GEN INTROSPECTOR

	basic_ltrb() : l(static_cast<T>(0)), t(static_cast<T>(0)), r(static_cast<T>(0)), b(static_cast<T>(0)) {}

	template <class B>
	basic_ltrb(const basic_xywh<B>& rr) : l(static_cast<T>(rr.x)), t(static_cast<T>(rr.y)), r(static_cast<T>(rr.x + rr.w)), b(static_cast<T>(rr.y + rr.h)) {}

	template <class B>
	basic_ltrb(const basic_ltrb<B>& rr) : l(static_cast<T>(rr.l)), t(static_cast<T>(rr.t)), r(static_cast<T>(rr.r)), b(static_cast<T>(rr.b)) {}

	basic_ltrb(const T l, const T t, const T r, const T b) : l(l), t(t), r(r), b(b) {}
	basic_ltrb(const basic_vec2<T> pos, const basic_vec2<T> size) : l(pos.x), t(pos.y), r(pos.x + size.x), b(pos.y + size.y) {}

	static basic_ltrb<T> from_points(
		const basic_vec2<T> a,
		const basic_vec2<T> b
	) {
		const auto lt = basic_vec2<T>(
			std::min(a.x, b.x),
			std::min(a.y, b.y)
		);

		const auto rt = basic_vec2<T>(
			std::max(a.x, b.x),
			std::max(a.y, b.y)
		);

		const auto size = rt - lt;

		return { lt, size };
	}

	static basic_ltrb<T> center_and_size(
		const vec2_type center, 
		const vec2_type size
	) {
		return { center - size / 2, size };
	}

	void x(const T xx) {
		*this += (basic_vec2<T>(xx - l, 0));
	}

	void y(const T yy) {
		*this += (basic_vec2<T>(0, yy - t));
	}

	void w(const T ww) {
		r = l + ww;
	}

	void h(const T hh) {
		b = t + hh;
	}

	template <class A = T, class = std::enable_if_t<std::is_floating_point_v<A>>>
	auto& round_fract() {
		l = repro::round(l);
		t = repro::round(t);
		r = repro::round(r);
		b = repro::round(b);

		return *this;
	}

	void set(const T _l, const T _t, const T _r, const T _b) {
		*this = basic_ltrb(_l, _t, _r, _b);
	}

	auto& set_position(const basic_vec2<T> v) {
		const auto old_w = w();
		const auto old_h = h();

		l = v.x;
		t = v.y;
		w(old_w);
		h(old_h);

		return *this;
	}

	auto& set_size(const basic_vec2<T> v) {
		w(v.x);
		h(v.y);

		return *this;
	}

	auto& set_size(const T x, const T y) {
		w(x);
		h(y);

		return *this;
	}

	auto& contain(const basic_ltrb rc) {
		if (!good()) {
			*this = rc;
			return *this;
		}

		l = std::min(l, rc.l);
		t = std::min(t, rc.t);
		r = std::max(r, rc.r);
		b = std::max(b, rc.b);

		return *this;
	}

	bool clip_by(const basic_ltrb rc) {
		if (l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t) {
			*this = basic_ltrb();
			return false;
		}
		*this = basic_ltrb(std::max(l, rc.l),
			std::max(t, rc.t),
			std::min(r, rc.r),
			std::min(b, rc.b));
		return true;
	}

	auto& expand_from_center(const basic_vec2<T> amount) {
		l -= amount.x;
		t -= amount.y;
		r += amount.x;
		b += amount.y;

		return *this;
	}

	template <class S>
	auto& expand_from_center_mult(const S scalar) {
		*this = center_and_size(get_center(), static_cast<basic_vec2<S>>(get_size()) * scalar);
		return *this;
	}

	auto& snap_to_bounds(const basic_ltrb rc) {
		basic_vec2<T> offset(0, 0);

		if (l < rc.l) offset.x += rc.l - l;
		if (r > rc.r) offset.x += rc.r - r;
		if (t < rc.t) offset.y += rc.t - t;
		if (b > rc.b) offset.y += rc.b - b;

		operator+=(offset);

		return *this;
	}

	auto& place_in_center_of(basic_ltrb bigger) {
		bigger.l += static_cast<T>(bigger.w() / 2.f - w() / 2.f);
		bigger.t += static_cast<T>(bigger.h() / 2.f - h() / 2.f);
		bigger.w(w());
		bigger.h(h());

		*this = bigger;
		return *this;
	}

	auto to_si_space(const si_scaling si) const {
		basic_ltrb result;
		result.l = si.get_meters(l);
		result.t = si.get_meters(t);
		result.r = si.get_meters(r);
		result.b = si.get_meters(b);
		return result;
	}

	auto to_user_space(const si_scaling si) const {
		basic_ltrb result;
		result.l = si.get_pixels(l);
		result.t = si.get_pixels(t);
		result.r = si.get_pixels(r);
		result.b = si.get_pixels(b);
		return result;
	}

	basic_ltrb& center_x(const T c) {
		const T _w = w();
		l = c - _w / 2;
		r = l + _w;

		return *this;
	}

	basic_ltrb& center_y(const T c) {
		const T _h = h();
		t = c - _h / 2;
		b = t + _h;

		return *this;
	}

	basic_ltrb& center(const basic_vec2<T> c) {
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

	basic_vec2<T> left_top() const {
		return { l, t };
	}

	basic_vec2<T> right_bottom() const {
		return { r, b };
	}

	basic_vec2<T> right_top() const {
		return { r, t };
	}

	basic_vec2<T> left_bottom() const {
		return { l, b };
	}

	basic_vec2<T> get_center() const {
		return (left_top() + right_bottom()) / 2;
	}

	T perimeter() const {
		return 2 * w() + 2 * h();
	}

	T max_side() const {
		return std::max(w(), h());
	}

	bool hover(const basic_ltrb& rc) const {
		return !(l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t);
	}

	bool hover(const basic_xywh<T>& rc) const {
		return hover(basic_ltrb(rc));
	}

	bool inside(const basic_ltrb& rc) const {
		return l >= rc.l && r <= rc.r && t >= rc.t && b <= rc.b;
	}

	basic_vec2<T> get_sticking_offset(const rectangle_sticking mode) const {
		basic_vec2<T> res;
		switch (mode) {
		case rectangle_sticking::LEFT: res = basic_vec2<T>(-r, 0);	break;
		case rectangle_sticking::RIGHT: res = basic_vec2<T>(-l, 0);	break;
		case rectangle_sticking::TOP: res = basic_vec2<T>(0, -b);	break;
		case rectangle_sticking::BOTTOM: res = basic_vec2<T>(0, -t);	break;
		default: res = basic_vec2<T>(0, 0); break;
		}

		return res;
	}

	template <typename S>
	bool hover(const basic_vec2<S>& m) const {
		return m.x >= l && m.y >= t && m.x <= r && m.y <= b;
	}

	template <class S>
	std::array<basic_vec2<S>, 4> get_vertices() const {
		return {
			basic_vec2<S>(l, t),
			basic_vec2<S>(r, t),
			basic_vec2<S>(r, b),
			basic_vec2<S>(l, b)
		};
	}

	template <typename type>
	void snap_point(basic_vec2<type>& v) const {
		if (v.x < l) v.x = static_cast<type>(l);
		if (v.y < t) v.y = static_cast<type>(t);
		if (v.x > r) v.x = static_cast<type>(r);
		if (v.y > b) v.y = static_cast<type>(b);
	}

	T area() const {
		return w()*h();
	}

	T diagonal() const {
		return repro::sqrt(w()*w() + h()*h());
	}

	bool good() const {
		return w() > 0 && h() > 0;
	}

	basic_vec2<T> get_position() const {
		return basic_vec2<T>(l, t);
	}

	basic_vec2<T> get_size() const {
		return basic_vec2<T>(w(), h());
	}

	template <class P>
	basic_ltrb& operator+=(const P& p) {
		l += T(p.x);
		t += T(p.y);
		r += T(p.x);
		b += T(p.y);
		return *this;
	}

	template <class P>
	basic_ltrb operator-(const P& p) const {
		return basic_ltrb(l - T(p.x), t - T(p.y), r - T(p.x), b - T(p.y));
	}

	template <class P>
	basic_ltrb operator+(const P& p) const {
		return basic_ltrb(l + T(p.x), t + T(p.y), r + T(p.x), b + T(p.y));
	}

	basic_ltrb& scale(const T s) {
		l *= s;
		t *= s;
		r *= s;
		b *= s;

		return *this;
	}

	bool operator==(const basic_ltrb& a) const {
		return l == a.l && r == a.r && t == a.t && b == a.b;
	}

	bool operator!=(const basic_ltrb& a) const {
		return l != a.l || r != a.r || t != a.t || b != a.b;
	}

	auto make_edges() const {
		std::array<typename vec2_type::segment_type, 4> result;

		result[0][0] = vec2_type(l, t);
		result[0][1] = vec2_type(r, t);

		result[1][0] = vec2_type(r, t);
		result[1][1] = vec2_type(r, b);

		result[2][0] = vec2_type(r, b);
		result[2][1] = vec2_type(l, b);

		result[3][0] = vec2_type(l, b);
		result[3][1] = vec2_type(l, t);

		return result;
	}

	auto make_vertices() const {
		std::array<vec2_type, 4> result;

		result[0] = vec2_type(l, t);
		result[1] = vec2_type(r, t);
		result[2] = vec2_type(r, b);
		result[3] = vec2_type(l, b);

		return result;
	}
};

template <class T>
struct basic_xywh {
	// GEN INTROSPECTOR struct basic_xywh class T
	T x;
	T y;
	T w;
	T h;
	// END GEN INTROSPECTOR

	using vec2_type = basic_vec2<T>;

	basic_xywh() : x(static_cast<T>(0)), y(static_cast<T>(0)), w(static_cast<T>(0)), h(static_cast<T>(0)) {}
	basic_xywh(const basic_ltrb<T> rc) : x(rc.l), y(rc.t) { b(rc.b); r(rc.r); }
	basic_xywh(const T x, const T y, const T w, const T h) : x(x), y(y), w(w), h(h) {}
	basic_xywh(const T x, const T y, const basic_vec2<T>& s) : x(x), y(y), w(s.x), h(s.y) {}
	basic_xywh(const basic_vec2<T>& p, const basic_vec2<T>& s) : x(p.x), y(p.y), w(s.x), h(s.y) {}

	static basic_xywh center_and_size(
		const vec2_type center, 
		const vec2_type size
	) {
		return { center - size / 2, size };
	}

	void set(const T _x, const T _y, const T _w, const T _h) {
		*this = basic_xywh(_x, _y, _w, _h);
	}

	auto& set_position(const basic_vec2<T> v) {
		x = v.x;
		y = v.y;

		return *this;
	}

	basic_vec2<T> get_position() const {
		return { x, y };
	}

	bool clip(const basic_xywh& rc) {
		if (x >= rc.r() || y >= rc.b() || r() <= rc.x || b() <= rc.y) {
			*this = basic_xywh();
			return false;
		}
		*this = basic_ltrb<T>(std::max(x, rc.x),
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

	T r() const {
		return x + this->w;
	};

	T b() const {
		return y + this->h;
	}

	template <class A = T, class = std::enable_if_t<std::is_floating_point_v<A>>>
	auto& round_fract() {
		x = repro::round(x);
		y = repro::round(y);
		w = repro::round(w);
		h = repro::round(h);

		return *this;
	}

	auto& set_size(const basic_vec2<T> v) {
		w = v.x;
		h = v.y;

		return *this;
	}

	template <class S>
	auto& expand_from_center_mult(const S scalar) {
		*this = center_and_size(get_center(), static_cast<basic_vec2<S>>(get_size()) * scalar);
		return *this;
	}

	auto& expand_from_center(const basic_vec2<T> amount) {
		basic_xywh result = *this;

		result.x -= amount.x;
		result.y -= amount.y;
		result.w += amount.x * 2;
		result.h += amount.y * 2;

		return result;
	}

	auto expand_to_square() const {
		basic_xywh result;
		result.w = std::max(w, h);
		result.h = result.w;

		result.x = x - (result.w - w) / 2;
		result.y = y - (result.h - h) / 2;

		return result;
	}

	basic_vec2<T> get_size() const {
		return{ w, h };
	}

	basic_vec2<T> get_center() const {
		return get_position() + get_size() / 2;
	}

	bool hover(const basic_vec2<T> m) const {
		return m.x >= x && m.y >= y && m.x <= r() && m.y <= b();
	}

	bool hover(const basic_ltrb<T> rc) const {
		return rc.hover(*this);
	}

	bool hover(const basic_xywh rc) const {
		return basic_ltrb<T>(rc).hover(*this);
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

	bool operator==(const basic_xywh r) const {
		return x == r.x && y == r.y && w == r.w && h == r.h;
	}

	bool operator!=(const basic_xywh r) const {
		return !operator==(r);
	}

	template <class P>
	basic_xywh& operator+=(const P& p) {
		x += T(p.x);
		y += T(p.y);
		return *this;
	}

	template <class P>
	basic_xywh operator-(const P& p) const {
		return basic_xywh(x - T(p.x), y - T(p.y), this->w, this->h);
	}

	template <class P>
	basic_xywh operator+(const P& p) const {
		return basic_xywh(x + T(p.x), y + T(p.y), this->w, this->h);
	}

	auto make_edges() const {
		std::array<typename vec2_type::segment_type, 4> result;

		result[0][0] = vec2_type(x, y);
		result[0][1] = vec2_type(x + w, y);

		result[1][0] = vec2_type(x + w, y);
		result[1][1] = vec2_type(x + w, y + h);

		result[2][0] = vec2_type(x + w, y + h);
		result[2][1] = vec2_type(x, y + h);

		result[3][0] = vec2_type(x, y + h);
		result[3][1] = vec2_type(x, y);

		return result;
	}

	auto make_vertices() const {
		std::array<vec2_type, 4> result;

		result[0] = vec2_type(x, y);
		result[1] = vec2_type(x + w, y);
		result[2] = vec2_type(x + w, y + h);
		result[3] = vec2_type(x, y + h);

		return result;
	}
};

template<class T>
std::ostream& operator<<(std::ostream& out, const basic_xywh<T>& x) {
	out << typesafe_sprintf("(xywh: %x;%x;%x;%x)", x.x, x.y, x.w, x.h);
	return out;
}

template<class T>
std::ostream& operator<<(std::ostream& out, const basic_ltrb<T>& x) {
	out << typesafe_sprintf("(ltrb: %x;%x;%x;%x)", x.l, x.t, x.r, x.b);
	return out;
}
