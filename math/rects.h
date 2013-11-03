#pragma once
#include <vector>
#include <sstream>
// trzeba usprawnic rect2D - rozszerzyc max_size na wh bo z samego max_s: wielkie prostokaty rozpychaja kwadrat a male wykorzystuja miejsce na dole

struct b2Vec2;

namespace augmentations {
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
			wh(const vec2<type>& rr) : w(static_cast<int>(rr.x)), h(static_cast<int>(rr.y)) {}
			wh(const ltrb&);
			wh(const xywh&);
			wh(int w = 0, int h = 0);

			enum class fit_status {
				DOESNT_FIT,
				FITS_INSIDE,
				FITS_INSIDE_FLIPPED,
				FITS_PERFECTLY,
				FITS_PERFECTLY_FLIPPED
			} fits(const wh& bigger) const;

			int w,h, area() const, perimeter() const, max_side() const;
			void stick_relative(const wh& content, vec2<float>& scroll) const;
			bool is_sticked(const wh& content, vec2<float>& scroll) const;
			bool inside(const wh& bigger) const;

			bool good() const;
			
			//wh& operator/=(int d) { w /= d; h /= d; return *this; }
			//wh& operator/=(float d) { w /= d; h /= d; return *this; }
			wh operator*(float) const;
			bool operator==(const wh&) const;
		};
		
		struct ltrb {
			ltrb();
			ltrb(const wh&);
			ltrb(const xywh&);
			ltrb(int left, int top, int right, int bottom);
			ltrb(const vec2<int>&, const wh& = wh());

			void contain(const ltrb& smaller);
			void contain_positive(const ltrb& smaller);

			bool clip(const ltrb& bigger);
			bool hover(const vec2<int>& mouse) const;
			bool hover(const ltrb&) const;
			bool hover(const xywh&) const;
			bool inside(const ltrb& bigger) const;
			
			bool stick_x(const ltrb& bigger);
			bool stick_y(const ltrb& bigger);
			
			vec2<float> center() const;
			void center_x(int x);
			void center_y(int y);
			void center(const vec2<int>&);

			template <typename type>
			void snap_point(vec2<type>& v) const {
				if (v.x < l) v.x = static_cast<type>(l);
				if (v.y < t) v.y = static_cast<type>(t);
				if (v.x > r) v.x = static_cast<type>(r);
				if (v.y > b) v.y = static_cast<type>(b);
			}

			int l, t, r, b, w() const, h() const, area() const, perimeter() const, max_side() const; // false - null rectangle
			void x(int), y(int), w(int), h(int);
			bool good() const;

			template <class P>
			ltrb& operator+=(const P& p) {
				l += int(p.x);
				t += int(p.y);
				r += int(p.x);
				b += int(p.y);
				return *this;
			}
			
			template <class P>
			ltrb operator-(const P& p) const {
				return ltrb(l - int(p.x), t - int(p.y), r - int(p.x), b - int(p.y));
			}

			template <class P>
			ltrb operator+(const P& p) const {
				return ltrb(l + int(p.x), t + int(p.y), r + int(p.x), b + int(p.y));
			}
		};

		struct xywh : public wh {
			xywh();
			xywh(const wh&);
			xywh(const ltrb&);
			xywh(int x, int y, int width, int height);
			xywh(int x, int y, const wh&);
			xywh(const vec2<int>&, const wh&);
			
			bool clip(const xywh& bigger); // false - null rectangle
			bool hover(const vec2<int>& mouse);
			bool hover(const xywh&);
			bool hover(const ltrb&);

			int x, y, r() const, b() const;
			void r(int), b(int);
			
			bool operator==(const xywh&) const;

			template <class P>
			xywh& operator+=(const P& p) {
				x += int(p.x);
				y += int(p.y);
				return *this;
			}
			
			template <class P>
			xywh operator-(const P& p) const {
				return xywh(x - int(p.x), y - int(p.y), w, h);
			}

			template <class P>
			xywh operator+(const P& p) const {
				return xywh(x + int(p.x), y + int(p.y), w, h);
			}
		};
		
		struct xywhf : public xywh {
			xywhf(const wh  &);
			xywhf(const ltrb&);
			xywhf(const xywh&);
			xywhf(int x, int y, int width, int height, bool flipped = false);
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