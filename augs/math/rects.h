#pragma once
#include <vector>
#include <sstream>
#include <algorithm>
#include "declare.h"
// trzeba usprawnic rect2D - rozszerzyc max_size na wh bo z samego max_s: wielkie prostokaty rozpychaja kwadrat a male wykorzystuja miejsce na dole

struct b2Vec2;

namespace augs {
	/* faciliates operations on rectangles and points */
	namespace rects {
		enum sticking {
			LEFT,
			RIGHT,
			TOP,
			BOTTOM
		};

		template <class T> struct wh;
		template <class T> struct ltrb;
		template <class T> struct xywh;

		template <class T>
		struct wh {
			template<typename type>
			wh(const vec2t<type>& rr) : w(static_cast<T>(rr.x)), h(static_cast<T>(rr.y)) {}
			wh(const ltrb<T>& rr) : w(rr.w()), h(rr.h()) {}
			wh(const xywh<T>& rr) : w(rr.w), h(rr.h) {}
			wh(T w = 0, T h = 0) : w(w), h(h) {}

			void set(T w, T h) { *this = wh(w, h); }

			enum class fit_status {
				DOESNT_FIT,
				FITS_INSIDE,
				FITS_INSIDE_FLIPPED,
				FITS_PERFECTLY,
				FITS_PERFECTLY_FLIPPED
			};
      
      fit_status fits(const wh& r) const {
				if (w == r.w && h == r.h) return fit_status::FITS_PERFECTLY;
				if (h == r.w && w == r.h) return fit_status::FITS_PERFECTLY_FLIPPED;
				if (w <= r.w && h <= r.h) return fit_status::FITS_INSIDE;
				if (h <= r.w && w <= r.h) return fit_status::FITS_INSIDE_FLIPPED;
				return fit_status::DOESNT_FIT;
			}

			T w, h;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(CEREAL_NVP(w), CEREAL_NVP(h));
			}
			
			void clamp_offset_to_right_down_corner_of(const wh& bigger, vec2t<T>& offset) const {
				offset.x = std::min(offset.x, T(bigger.w - w));
				offset.x = std::max(offset.x, 0.f);
				offset.y = std::min(offset.y, T(bigger.h - h));
				offset.y = std::max(offset.y, 0.f);
			}

			bool inside(const wh& rc) const {
				return w <= rc.w && h <= rc.h;
			}

			bool can_contain(const wh& another_rect, const vec2t<T>& offset) const {
				return offset.x >= 0.f && offset.x <= another_rect.w - w && offset.y >= 0 && offset.y <= another_rect.h - h;
			}

			bool good() const {
				return w > 0 && h > 0;
			}

			wh operator*(T s) const {
				return wh(T(w*s), T(h*s));
			}

			bool operator==(const wh& r) const {
				return w == r.w && h == r.h;
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
		};
		
		template <class T>
		struct ltrb {
			T l, t, r, b;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(CEREAL_NVP(l), CEREAL_NVP(t), CEREAL_NVP(r), CEREAL_NVP(b));
			}

			ltrb() : l(0), t(0), r(0), b(0) {}
			ltrb(const wh<T>& rr) : l(0), t(0), r(rr.w), b(rr.h) {}

			template <class B>
			ltrb(const xywh<B>& rr) : l(static_cast<T>(rr.x)), t(static_cast<T>(rr.y)), r(static_cast<T>(rr.x + rr.w)), b(static_cast<T>(rr.y + rr.h)) {}
			
			template <class B>
			ltrb(const ltrb<B>& rr) : l(static_cast<T>(rr.l)), t(static_cast<T>(rr.t)), r(static_cast<T>(rr.r)), b(static_cast<T>(rr.b)) {}
			
			ltrb(T l, T t, T r, T b) : l(l), t(t), r(r), b(b) {}
			ltrb(vec2t<T> pos, vec2t<T> size) : l(pos.x), t(pos.y), r(pos.x + size.x), b(pos.y + size.y) {}
			void set(T l, T t, T r, T b) { *this = ltrb(l, t, r, b); }

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

			void contain(const ltrb& rc) {
				l = std::min(l, rc.l);
				t = std::min(t, rc.t);
				contain_positive(rc);
			}

			void contain_positive(const ltrb& rc) {
				r = std::max(r, rc.r);
				b = std::max(b, rc.b);
			}

			bool clip_by(const ltrb& rc) {
				if (l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t) {
					*this = ltrb();
					return false;
				}
				*this = ltrb(std::max(l, rc.l),
					std::max(t, rc.t),
					std::min(r, rc.r),
					std::min(b, rc.b));
				return true;
			}

			bool hover(const ltrb& rc) const {
				return !(l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t);
			}

			bool hover(const xywh<T>& rc) const {
				return hover(ltrb(rc));
			}

			bool inside(const ltrb& rc) const {
				return l >= rc.l && r <= rc.r && t >= rc.t && b <= rc.b;
			}

			ltrb& expand_from_center(const vec2 amount) {
				l -= amount.x;
				t -= amount.y;
				r += amount.x;
				b += amount.y;

				return *this;
			}

			ltrb& snap_to_bounds(const ltrb& rc) {
				vec2t<T> offset(0, 0);

				if (l < rc.l) offset.x += rc.l - l;
				if (r > rc.r) offset.x += rc.r - r;
				if (t < rc.t) offset.y += rc.t - t;
				if (b > rc.b) offset.y += rc.b - b;

				operator+=(offset);

				return *this;
			}

			vec2t<T> get_sticking_offset(sticking mode) const {
				vec2t<T> res;
				switch (mode) {
				case sticking::LEFT: res = vec2t<T>(-r, 0);	break;
				case sticking::RIGHT: res = vec2t<T>(-l, 0);	break;
				case sticking::TOP: res = vec2t<T>(0, -b);	break;
				case sticking::BOTTOM: res = vec2t<T>(0, -t);	break;
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

			T w() const {
				return r - l;
			}
			
			T h() const {
				return b - t;
			}

			T area() const {
				return w()*h();
			}

			void center_x(T c) {
				T _w = w();
				l = c - _w / 2;
				r = l + _w;
			}

			void center_y(T c) {
				T _h = h();
				t = c - _h / 2;
				b = t + _h;
			}

			void center(const vec2t<T>& c) {
				center_x(c.x);
				center_y(c.y);
			}

			void x(T xx) {
				*this += (vec2t<T>(xx - l, 0));
			}

			void y(T yy) {
				*this += (vec2t<T>(0, yy - t));
			}

			void w(T ww) {
				r = l + ww;
			}

			void h(T hh) {
				b = t + hh;
			}

			bool good() const {
				return w() > 0 && h() > 0;
			}

			vec2t<T> get_position() const {
				return vec2t<T>(l, t);
			}

			void set_position(vec2t<T> v) {
				auto old_w = w();
				auto old_h = h();

				l = v.x;
				t = v.y;
				w(old_w);
				h(old_h);
			}

			void set_size(vec2t<T> v) {
				w(v.x);
				h(v.y);
			}

			void set_size(T x, T y) {
				w(x);
				h(y);
			}

			vec2t<T> get_size() const {
				return vec2t<T>(w(), h());
			}

			template <class P>
			ltrb& operator+=(const P& p) {
				l += T(p.x);
				t += T(p.y);
				r += T(p.x);
				b += T(p.y);
				return *this;
			}
		
			template <class P>
			ltrb operator-(const P& p) const {
				return ltrb(l - T(p.x), t - T(p.y), r - T(p.x), b - T(p.y));
			}

			template <class P>
			ltrb operator+(const P& p) const {
				return ltrb(l + T(p.x), t + T(p.y), r + T(p.x), b + T(p.y));
			}

			ltrb& scale(T s) {
				l *= s;
				t *= s;
				r *= s;
				b *= s;

				return *this;
			}

			bool operator==(const ltrb& a) {
				return l == a.l && r == a.r && t == a.t && b == a.b;
			}
		};

		template <class T>
		struct xywh : public wh<T> {
			xywh() : x(0), y(0) {}
			xywh(const wh<T>& rr) : x(0), y(0), wh<T>(rr) {}
			xywh(const ltrb<T>& rc) : x(rc.l), y(rc.t) { b(rc.b); r(rc.r); }
			xywh(T x, T y, T w, T h) : x(x), y(y), wh<T>(w, h) {}
			xywh(T x, T y, const wh<T>& r) : x(x), y(y), wh<T>(r) {}
			xywh(const vec2t<T>& p, const wh<T>& r) : x(p.x), y(p.y), wh<T>(r) {}

			void set(T x, T y, T w, T h) { *this = xywh(x, y, w, h); }
			
			vec2t<T> center() const {
				return{ x + this->w/2, y + this->h/2 };
			}

			vec2t<T> get_size() const {
				return{ w, h };
			}

			bool clip(const xywh& rc) {
				if (x >= rc.r() || y >= rc.b() || r() <= rc.x || b() <= rc.y) {
					*this = xywh();
					return false;
				}
				*this = ltrb<T>(std::max(x, rc.x),
					std::max(y, rc.y),
					std::min(r(), rc.r()),
					std::min(b(), rc.b()));
				return true;
			}

			bool hover(const vec2t<T>& m) const {
				return m.x >= x && m.y >= y && m.x <= r() && m.y <= b();
			}

			bool hover(const ltrb<T>& rc) const {
				return rc.hover(*this);
			}

			bool hover(const xywh& rc) const {
				return ltrb<T>(rc).hover(*this);
			}

			T r() const {
				return x + this->w;
			};

			T b() const {
				return y + this->h;
			}

			void r(T right) {
				this->w = right - x;
			}

			void b(T bottom) {
				this->h = bottom - y;
			}

			xywh& expand_from_center(const vec2 amount) {
				x -= amount.x;
				y -= amount.y;
				w += amount.x;
				h += amount.y;
				
				return *this;
			}

			T x, y;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(w), CEREAL_NVP(h));
			}

			bool operator==(const xywh& r) const {
				return x == r.x && y == r.y && wh<T>::operator==(r);
			}

			template <class P>
			xywh& operator+=(const P& p) {
				x += T(p.x);
				y += T(p.y);
				return *this;
			}
			
			template <class P>
			xywh operator-(const P& p) const {
				return xywh(x - T(p.x), y - T(p.y), this->w, this->h);
			}

			template <class P>
			xywh operator+(const P& p) const {
				return xywh(x + T(p.x), y + T(p.y), this->w, this->h);
			}
		};
		
		template <class T>
		struct xywhf : public xywh<T> {
			xywhf(const ltrb<T>& rr) : xywh<T>(rr), flipped(false) {}
			xywhf(const xywh<T>& rr) : xywh<T>(rr), flipped(false) {}
			xywhf(const wh<T>  & rr) : xywh<T>(rr), flipped(false) {}
			xywhf(T x, T y, T width, T height, bool flipped) : xywh<T>(x, y, width, height), flipped(flipped) {}
			xywhf() : flipped(false) {}

			void flip() {
				flipped = !flipped;
				std::swap(this->w, this->h);
			}

			xywh<T> rc() const {
				return xywh<T>(this->x, this->y, flipped ? this->h : this->w, flipped ? this->w : this->h);
			}

			bool flipped;
		};

		template <class T>
		struct texture {
			T u1, v1, u2, v2;
			texture(T u1 = 1.0, T v1 = 1.0, T u2 = 1.0, T v2 = 1.0) : u1(u1), v1(v1), u2(u2), v2(v2) {}
		};

		template <class T>
		struct point_texture {
			T u, v;
		};

		template <class T>
		extern std::wostream& operator<<(std::wostream&, const vec2t<T>&);

		template <class T>
		extern std::ostream& operator<<(std::ostream&, const vec2t<T>&);
	}
}

using namespace augs;