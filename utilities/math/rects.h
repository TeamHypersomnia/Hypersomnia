#pragma once
#include <vector>
#include <sstream>
// trzeba usprawnic rect2D - rozszerzyc max_size na wh bo z samego max_s: wielkie prostokaty rozpychaja kwadrat a male wykorzystuja miejsce na dole

struct b2Vec2;

namespace augs {
	namespace window {
		class glwindow;
	}
		
	template<typename type> struct vec2;

	/* faciliates operations on rectangles and points */
	namespace rects {
		struct wh;
		struct ltrb;
		struct xywh;

		struct wh {
			template<typename type>
			wh(const vec2<type>& rr) : w(static_cast<float>(rr.x)), h(static_cast<float>(rr.y)) {}
			wh(const ltrb&);
			wh(const xywh&);
			wh(float w = 0, float h = 0);

			enum class fit_status {
				DOESNT_FIT,
				FITS_INSIDE,
				FITS_INSIDE_FLIPPED,
				FITS_PERFECTLY,
				FITS_PERFECTLY_FLIPPED
			} fits(const wh& bigger) const;

			float w,h, area() const, perimeter() const, max_side() const;
			void stick_relative(const wh& content, vec2<float>& scroll) const;
			bool is_sticked(const wh& content, vec2<float>& scroll) const;
			bool inside(const wh& bigger) const;

			bool good() const;
			
			//wh& operator/=(float d) { w /= d; h /= d; return *this; }
			//wh& operator/=(float d) { w /= d; h /= d; return *this; }
			wh operator*(float) const;
			bool operator==(const wh&) const;
		};
		
		struct ltrb {
			ltrb();
			ltrb(const wh&);
			ltrb(const xywh&);
			ltrb(float left, float top, float right, float bottom);
			ltrb(const vec2<float>&, const wh& = wh());

			void contain(const ltrb& smaller);
			void contain_positive(const ltrb& smaller);

			bool clip(const ltrb& bigger);
			
			template <typename T>
			bool hover(const vec2<T>& m) const {
				return m.x >= l && m.y >= t && m.x <= r && m.y <= b;
			}

			template <class T>
			static ltrb get_aabb(vec2<T>* v) {
				auto x_pred = [](vec2<T> a, vec2<T> b){ return a.x < b.x; };
				auto y_pred = [](vec2<T> a, vec2<T> b){ return a.y < b.y; };

				vec2<T> lower(
					static_cast<T>(std::min_element(v, v + 4, x_pred)->x),
					static_cast<T>(std::min_element(v, v + 4, y_pred)->y)
					);

				vec2<T> upper(
					static_cast<T>(std::max_element(v, v + 4, x_pred)->x),
					static_cast<T>(std::max_element(v, v + 4, y_pred)->y)
					);

				return rects::ltrb(lower.x, lower.y, upper.x, upper.y);
			}

			template <class T>
			std::vector<vec2<T>> get_vertices() const {
				std::vector<vec2<T>> out;
				out.push_back(vec2<T>(l, t));
				out.push_back(vec2<T>(r, t));
				out.push_back(vec2<T>(r, b));
				out.push_back(vec2<T>(l, b));
				return std::move(out);
			}

			bool hover(const ltrb&) const;
			bool hover(const xywh&) const;
			bool inside(const ltrb& bigger) const;
			
			bool stick_x(const ltrb& bigger);
			bool stick_y(const ltrb& bigger);
			
			vec2<float> center() const;
			void center_x(float x);
			void center_y(float y);
			void center(const vec2<float>&);

			template <typename type>
			void snap_point(vec2<type>& v) const {
				if (v.x < l) v.x = static_cast<type>(l);
				if (v.y < t) v.y = static_cast<type>(t);
				if (v.x > r) v.x = static_cast<type>(r);
				if (v.y > b) v.y = static_cast<type>(b);
			}

			float l, t, r, b, w() const, h() const, area() const, perimeter() const, max_side() const; // false - null rectangle
			void x(float), y(float), w(float), h(float);
			bool good() const;

			template <class P>
			ltrb& operator+=(const P& p) {
				l += float(p.x);
				t += float(p.y);
				r += float(p.x);
				b += float(p.y);
				return *this;
			}
			
			template <class P>
			ltrb operator-(const P& p) const {
				return ltrb(l - float(p.x), t - float(p.y), r - float(p.x), b - float(p.y));
			}

			template <class P>
			ltrb operator+(const P& p) const {
				return ltrb(l + float(p.x), t + float(p.y), r + float(p.x), b + float(p.y));
			}
		};

		struct xywh : public wh {
			xywh();
			xywh(const wh&);
			xywh(const ltrb&);
			xywh(float x, float y, float width, float height);
			xywh(float x, float y, const wh&);
			xywh(const vec2<float>&, const wh&);
			
			bool clip(const xywh& bigger); // false - null rectangle
			bool hover(const vec2<float>& mouse);
			bool hover(const xywh&);
			bool hover(const ltrb&);

			float x, y, r() const, b() const;
			void r(float), b(float);
			
			bool operator==(const xywh&) const;

			template <class P>
			xywh& operator+=(const P& p) {
				x += float(p.x);
				y += float(p.y);
				return *this;
			}
			
			template <class P>
			xywh operator-(const P& p) const {
				return xywh(x - float(p.x), y - float(p.y), w, h);
			}

			template <class P>
			xywh operator+(const P& p) const {
				return xywh(x + float(p.x), y + float(p.y), w, h);
			}
		};
		
		struct xywhf : public xywh {
			xywhf(const wh  &);
			xywhf(const ltrb&);
			xywhf(const xywh&);
			xywhf(float x, float y, float width, float height, bool flipped = false);
			xywhf();
			void flip();
			xywh rc() const;
			bool flipped;
		};

		struct texture {
			float u1, v1, u2, v2;
			texture(float u1 = 1.0, float v1 = 1.0, float u2 = 1.0, float v2 = 1.0);
		};

		struct point_texture {
			float u, v;
		};

		extern std::wostream& operator<<(std::wostream&, const vec2<int>&);
		extern std::wostream& operator<<(std::wostream&, const vec2<float>&);
		extern std::ostream& operator<<(std::ostream&, const vec2<int>&);
		extern std::ostream& operator<<(std::ostream&, const vec2<float>&);
	}
}