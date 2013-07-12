#pragma once
#include <vector>
#include <sstream>
// trzeba usprawnic rect2D - rozszerzyc max_size na wh bo z samego max_s: wielkie prostokaty rozpychaja kwadrat a male wykorzystuja miejsce na dole

namespace augmentations {
	namespace window {
		class glwindow;
	}
	/* faciliates operations on rectangles and points */
	namespace rects {
		struct ltrb;
		struct xywh;
		struct point;
		struct pointf;

		struct point {
			int x, y;
			point(int x = 0, int y = 0);
			point(const ltrb&);
			point(const xywh&);
			point(const pointf&);
			point operator-() const;
			point operator-(const point&) const;
			pointf operator-(const pointf&) const;
			point operator+(const point&) const;
			pointf operator+(const pointf&) const;
			point operator*(const point&) const;
			pointf operator*(const pointf&) const;
			point& operator-=(const point&);
			point& operator-=(const pointf&);
			point& operator+=(const point&);
			point& operator+=(const pointf&);
			point& operator*=(const point&);
			point& operator*=(const pointf&);
			point& operator*=(int);
			point& operator*=(float);
			pointf operator*(float) const;
		};

		double deg2rad(double);

		struct pointf {
			float x, y;
			pointf(const point&);
			pointf(const ltrb&);
			pointf(const xywh&);
			pointf(float x = 0.f, float y = 0.f);
			void normalize();
			float length() const;
			
			pointf operator-(const point&) const;
			pointf operator-(const pointf&) const;
			pointf operator+(const point&) const;
			pointf operator+(const pointf&) const;
			pointf operator*(const point&) const;
			pointf operator*(const pointf&) const;
			pointf& operator-=(const point&);
			pointf& operator-=(const pointf&);
			pointf& operator+=(const point&);
			pointf& operator+=(const pointf&);
			pointf& operator*=(const point&);
			pointf& operator*=(const pointf&);
			pointf operator*(float) const;
		};

		struct wh {
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
			void stick_relative(const wh& content, pointf& scroll) const;
			bool is_sticked(const wh& content, pointf& scroll) const;
			bool inside(const wh& bigger) const;

			bool good() const;
			wh operator*(float) const;
			bool operator==(const wh&) const;
		};
		
		struct ltrb {
			ltrb();
			ltrb(const wh&);
			ltrb(const xywh&);
			ltrb(int left, int top, int right, int bottom);
			ltrb(const point&, const wh& = wh());

			void contain(const ltrb& smaller);
			void contain_positive(const ltrb& smaller);

			bool clip(const ltrb& bigger);
			bool hover(const point& mouse) const;
			bool hover(const ltrb&) const;
			bool inside(const ltrb& bigger) const;
			
			bool stick_x(const ltrb& bigger);
			bool stick_y(const ltrb& bigger);
			
			pointf center() const;
			void center_x(int x);
			void center_y(int y);
			void center(const point&);

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
			xywh(const point&, const wh&);
			
			bool clip(const xywh& bigger); // false - null rectangle
			bool hover(const point& mouse);

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

		extern std::wostream& operator<<(std::wostream&, const point&);
		extern std::wostream& operator<<(std::wostream&, const pointf&);
		extern std::ostream& operator<<(std::ostream&, const point&);
		extern std::ostream& operator<<(std::ostream&, const pointf&);
	}
}