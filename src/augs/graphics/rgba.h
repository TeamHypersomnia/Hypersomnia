#pragma once
#include <array>
#include <sstream>
#include <algorithm>
#include "augs/build_settings/compiler_defines.h"

struct ImVec4;
struct hsv;
struct hsl;

using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;
using rgba_channel = unsigned char;

struct hsl {
	int h;
	float s;
	float l;

	hsl(int h, float s, float l) : h(h), s(s), l(l) {}
};

struct rgba {
	struct rgb_type {
		rgba_channel r;
		rgba_channel g;
		rgba_channel b;

		rgb_type(
			const rgba_channel red = 255,
			const rgba_channel green = 255,
			const rgba_channel blue = 255
		);

		rgb_type(const vec3&);
		operator vec3() const;
	};

	static rgba zero;

	// GEN INTROSPECTOR struct rgba
	rgba_channel r;
	rgba_channel g;
	rgba_channel b;
	rgba_channel a;
	// END GEN INTROSPECTOR

	rgba() {} /* Non-initializing constructor for performance */
	rgba(const vec4&);
	rgba(const ImVec4&);

	rgba(
		const hsv,
		const rgba_channel alpha = 255
	);

	rgba(
		const rgb_type, 
		const rgba_channel alpha = 255
	);

	rgba(
		const rgba_channel red, 
		const rgba_channel green, 
		const rgba_channel blue, 
		const rgba_channel alpha
	);

	operator ImVec4() const;
	operator vec4() const;

	void set(
		const rgba_channel red, 
		const rgba_channel green, 
		const rgba_channel blue, 
		const rgba_channel alpha
	);

	void set(const rgba);

	rgba& mult_alpha(float);
	rgba& multiply_rgb(float);

	rgba operator*(const float) const;
	rgba operator+(const rgba b) const;
	rgba operator-(const rgba b) const;
	rgba operator*(const rgba b) const;
	rgba& operator*=(const rgba b);
	rgba& operator+=(const rgba b);

	bool operator==(const rgba b) const;
	bool operator!=(const rgba b) const;
	hsv get_hsv() const;
	hsl get_hsl() const;
	rgba& desaturate();

	rgba_channel& operator[](const size_t index);
	const rgba_channel& operator[](const size_t index) const;

	rgba& set_rgb(const rgb_type&);
	rgba& set_hsv(const hsv);
	rgba& set_hsl(const hsl);

	rgba& mult_luminance(const float scalar);

	rgb_type rgb() const;

	void avoid_dark_blue_for_color_wave();

	template <class T>
	T& stream_to(T& out) const {
		const int ir = r;
		const int ig = g;
		const int ib = b;
		const int ia = a;

		out << ir << " " << ig << " " << ib << " " << ia;
		return out;
	}

	template <class T>
	T& from_stream(T& in) {
		int ir;
		int ig;
		int ib;
		int ia;

		in >> ir >> ig >> ib >> ia;

		r = static_cast<rgba_channel>(ir);
		g = static_cast<rgba_channel>(ig);
		b = static_cast<rgba_channel>(ib);
		a = static_cast<rgba_channel>(ia);

		return in;
	}

	friend std::ostream& operator<<(std::ostream& out, const rgba& x);
	friend std::istream& operator>>(std::istream& out, rgba& x);
};

struct hsv {
	double h;       // 0-1
	double s;       // 0-1
	double v;       // 0-1
	hsv(double = 0.0, double = 0.0, double = 0.0);
	hsv operator*(float) const;
	hsv operator+(hsv b) const;

	explicit operator rgba::rgb_type() const;
};

inline auto to_0_1(const rgba_channel c) {
	return c / 255.f;
}

inline auto to_0_255(const float c) {
	return static_cast<rgba_channel>(c * 255.f);
}

inline auto to_0_255(const double c) {
	return static_cast<rgba_channel>(c * 255);
}

namespace rgba_detail {
	struct rgb {
		double r;       // percent
		double g;       // percent
		double b;       // percent
	};

	hsv rgb2hsv(const rgb in);
	rgb hsv2rgb(const hsv in);
}

FORCE_INLINE hsv::operator rgba::rgb_type() const {
	const auto res = rgba_detail::hsv2rgb({ h * 360, s, v });
	
	return { 
		to_0_255(res.r), 
		to_0_255(res.g), 
		to_0_255(res.b) 
	};
}

FORCE_INLINE rgba::rgb_type::rgb_type(
	const rgba_channel red,
	const rgba_channel green,
	const rgba_channel blue
) :
	r(red),
	g(green),
	b(blue)
{}

FORCE_INLINE rgba::rgb_type::rgb_type(const vec3& v) :
	rgb_type(
		to_0_255(v[0]),
		to_0_255(v[1]),
		to_0_255(v[2])
	)
{}

FORCE_INLINE rgba::rgb_type::operator vec3() const {
	return {
		to_0_1(r),
		to_0_1(g),
		to_0_1(b)
	};
}

FORCE_INLINE rgba::rgba(const vec4& v) :
	rgba(
		to_0_255(v[0]),
		to_0_255(v[1]),
		to_0_255(v[2]),
		to_0_255(v[3])
	)
{}

FORCE_INLINE rgba::operator vec4() const {
	return {
		to_0_1(r),
		to_0_1(g),
		to_0_1(b),
		to_0_1(a)
	};
}

FORCE_INLINE rgba::rgba(
	const rgba_channel red, 
	const rgba_channel green, 
	const rgba_channel blue, 
	const rgba_channel alpha
) : 
	r(red), 
	g(green), 
	b(blue), 
	a(alpha) 
{}

FORCE_INLINE rgba::rgba(
	const rgb_type rgb,
	const rgba_channel alpha
) :
	r(rgb.r),
	g(rgb.g),
	b(rgb.b),
	a(alpha) 
{}

FORCE_INLINE rgba::rgba(
	const hsv h,
	const rgba_channel alpha
) : rgba(rgb_type(h), alpha) 
{}

FORCE_INLINE hsv::hsv(
	const double h, 
	const double s, 
	const double v
) : 
	h(h), 
	s(s), 
	v(v) 
{}

FORCE_INLINE hsv hsv::operator*(const float x) const {
	return hsv(h * x, s * x, v * x);
}

FORCE_INLINE hsv hsv::operator+(const hsv b) const {
	return hsv(h + b.h, s + b.s, v + b.v);
}

FORCE_INLINE void rgba::set(
	const rgba_channel red, 
	const rgba_channel green, 
	const rgba_channel blue, 
	const rgba_channel alpha
) {
	*this = rgba(
		red, 
		green, 
		blue, 
		alpha
	);
}

FORCE_INLINE void rgba::set(const rgba col) {
	*this = col;
}

FORCE_INLINE rgba& rgba::mult_alpha(const float s) {
	a = static_cast<rgba_channel>(s * a);
	return *this;
}

FORCE_INLINE rgba& rgba::multiply_rgb(const float s) {
	r = static_cast<rgba_channel>(std::clamp(s * r, 0.f, 255.f));
	g =	static_cast<rgba_channel>(std::clamp(s * g, 0.f, 255.f));
	b =	static_cast<rgba_channel>(std::clamp(s * b, 0.f, 255.f));

	return *this;
}

FORCE_INLINE rgba rgba::operator*(const rgba s) const {
	return rgba(
		static_cast<rgba_channel>(to_0_1(s.r) * r),
		static_cast<rgba_channel>(to_0_1(s.g) * g),
		static_cast<rgba_channel>(to_0_1(s.b) * b),
		static_cast<rgba_channel>(to_0_1(s.a) * a)
	);
}

FORCE_INLINE rgba rgba::operator*(const float s) const {
	return rgba(
		static_cast<rgba_channel>(s * r),
		static_cast<rgba_channel>(s * g),
		static_cast<rgba_channel>(s * b),
		static_cast<rgba_channel>(s * a)
	);
}

FORCE_INLINE rgba rgba::operator+(const rgba s) const {
	return rgba(
		std::min(255u, static_cast<unsigned>(s.r) + r),
		std::min(255u, static_cast<unsigned>(s.g) + g),
		std::min(255u, static_cast<unsigned>(s.b) + b),
		std::min(255u, static_cast<unsigned>(s.a) + a)
	);
}

FORCE_INLINE rgba rgba::operator-(const rgba s) const {
	return rgba(
		std::max(0, static_cast<int>(r) - static_cast<int>(s.r)),
		std::max(0, static_cast<int>(g) - static_cast<int>(s.g)),
		std::max(0, static_cast<int>(b) - static_cast<int>(s.b)),
		std::max(0, static_cast<int>(a) - static_cast<int>(s.a))
	);
}

FORCE_INLINE rgba& rgba::operator*=(const rgba s) {
	return (*this = *this * s);
}

FORCE_INLINE rgba& rgba::operator+=(const rgba s) {
	return (*this = *this + s);
}

FORCE_INLINE bool rgba::operator==(const rgba v) const {
	return r == v.r && g == v.g && b == v.b && a == v.a;
}

FORCE_INLINE bool rgba::operator!=(const rgba v) const {
	return !operator==(v);
}

FORCE_INLINE hsv rgba::get_hsv() const {
	auto res = rgba_detail::rgb2hsv({ r / 255.0, g / 255.0, b / 255.0 });
	return{ res.h / 360.0, res.s, res.v };
}

FORCE_INLINE rgba& rgba::desaturate() {
	const auto avg = static_cast<rgba_channel>((static_cast<unsigned>(r) + g + b) / 3u);
	r = avg;
	g = avg;
	b = avg;
	return *this;
}

FORCE_INLINE rgba_channel& rgba::operator[](const size_t index) {
	return (&r)[index];
}

FORCE_INLINE const rgba_channel& rgba::operator[](const size_t index) const {
	return (&r)[index];
}

FORCE_INLINE rgba& rgba::set_rgb(const rgb_type& right) {
	r = right.r;
	g = right.g;
	b = right.b;
	return *this;
}

FORCE_INLINE rgba::rgb_type rgba::rgb() const {
	return { r, g, b };
}

FORCE_INLINE rgba& rgba::set_hsv(const hsv hsv) {
	set_rgb(hsv.operator rgb_type());
	return *this;
}

inline const rgba maroon(0x80, 0x00, 0x00, 0xff);
inline const rgba red(0xff, 0x00, 0x00, 0xff);
inline const rgba orange(0xff, 0xA5, 0x00, 0xff);
inline const rgba yellow(0xff, 0xff, 0x00, 0xff);
inline const rgba olive(0x80, 0x80, 0x00, 0xff);
inline const rgba purple(0x80, 0x00, 0x80, 0xff);
inline const rgba fuchsia(0xff, 0x00, 0xff, 0xff);
inline const rgba white(0xff, 0xff, 0xff, 0xff);
inline const rgba lime(0x00, 0xff, 0x00, 0xff);
inline const rgba green(0, 255, 0, 255); //(0x00, 0x80, 0x00, 0xff);
inline const rgba navy(0x00, 0x00, 0x80, 0xff);
inline const rgba blue(0x00, 0x00, 0xff, 0xff);
inline const rgba aqua(0x00, 0xff, 0xff, 0xff);
inline const rgba teal(0x00, 0x80, 0x80, 0xff);
inline const rgba black(0x00, 0x00, 0x00, 0xff);
inline const rgba silver(0xc0, 0xc0, 0xc0, 0xff);
inline const rgba gray(0x80, 0x80, 0x80, 0xff);

inline const rgba ltblue(0, 122, 204, 255);
inline const rgba dark_green(0, 144, 66, 255);
inline const rgba pink(255, 0, 255, 255);
inline const rgba violet(164, 68, 195, 255);
inline const rgba red_violet(200, 68, 195, 255);
inline const rgba darkred(122, 0, 0, 255);
inline const rgba darkgray(30, 30, 30, 255);
inline const rgba gray1(50, 50, 50, 255);
inline const rgba gray2(62, 62, 62, 255);
inline const rgba gray3(104, 104, 104, 255);
inline const rgba gray4(180, 180, 180, 255);
inline const rgba slightly_visible_white(255, 255, 255, 15);
inline const rgba darkblue(6, 5, 20, 255);

inline const rgba cyan(0, 255, 255, 255);

inline const rgba vsgreen(87, 166, 74, 255);
inline const rgba vsdarkgreen(0, 100, 0, 255);
inline const rgba vscyan(78, 201, 176, 255);
inline const rgba vsblue(86, 156, 214, 255);

inline const rgba vsyellow(181, 206, 168, 255);
inline const rgba vslightgray(220, 220, 220, 255);
inline const rgba vsdarkgray(127, 127, 127, 255);

inline const rgba turquoise(0, 146, 222, 255);
