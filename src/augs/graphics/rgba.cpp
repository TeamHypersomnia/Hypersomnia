#include <limits>
#include <array>
#include <algorithm>
#include <imgui/imgui.h>
#include "augs/graphics/rgba.h"

namespace {
	typedef struct {
		double r;       // percent
		double g;       // percent
		double b;       // percent
	} rgb;
}

static hsv rgb2hsv(const rgb in) {
	hsv         out;
	double      min, max, delta;

	min = in.r < in.g ? in.r : in.g;
	min = min < in.b ? min : in.b;

	max = in.r > in.g ? in.r : in.g;
	max = max > in.b ? max : in.b;

	out.v = max;                                // v
	delta = max - min;
	if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
		out.s = (delta / max);                  // s
	}
	else {
		// if max is 0, then r = g = b = 0              
		// s = 0, v is undefined
		out.s = 0.0;
		out.h = std::numeric_limits<double>::quiet_NaN();                            // its now undefined
		return out;
	}
	if (in.r >= max)                           // > is bogus, just keeps compilor happy
		out.h = (in.g - in.b) / delta;        // between yellow & magenta
	else
		if (in.g >= max)
			out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
		else
			out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

	out.h *= 60.0;                              // degrees

	if (out.h < 0.0)
		out.h += 360.0;

	return out;
}

static rgb hsv2rgb(const hsv in) {
	double      hh, p, q, t, ff;
	long        i;
	rgb         out;

	if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));

	switch (i) {
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}

hsv::operator rgba::rgb_type() const {
	const auto res = hsv2rgb({ h * 360, s, v });
	
	return { 
		to_0_255(res.r), 
		to_0_255(res.g), 
		to_0_255(res.b) 
	};
}

rgba::rgb_type::rgb_type(
	const rgba_channel red,
	const rgba_channel green,
	const rgba_channel blue
) :
	r(red),
	g(green),
	b(blue)
{}

rgba::rgb_type::rgb_type(const vec3& v) :
	rgb_type(
		to_0_255(v[0]),
		to_0_255(v[1]),
		to_0_255(v[2])
	)
{}

rgba::rgb_type::operator vec3() const {
	return {
		to_0_1(r),
		to_0_1(g),
		to_0_1(b)
	};
}

rgba::rgba(const vec4& v) :
	rgba(
		to_0_255(v[0]),
		to_0_255(v[1]),
		to_0_255(v[2]),
		to_0_255(v[3])
	)
{}

rgba::operator vec4() const {
	return {
		to_0_1(r),
		to_0_1(g),
		to_0_1(b),
		to_0_1(a)
	};
}

rgba::rgba(const ImVec4& v) :
	rgba(
		to_0_255(v.x),
		to_0_255(v.y),
		to_0_255(v.z),
		to_0_255(v.w)
	)
{}

rgba::operator ImVec4() const {
	return { 
		to_0_1(r),
		to_0_1(g),
		to_0_1(b),
		to_0_1(a)
	};
}

rgba::rgba(
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

rgba::rgba(
	const rgb_type rgb,
	const rgba_channel alpha
) :
	r(rgb.r),
	g(rgb.g),
	b(rgb.b),
	a(alpha) 
{}

rgba::rgba(
	const hsv h,
	const rgba_channel alpha
) : rgba(rgb_type(h), alpha) 
{}

hsv::hsv(
	const double h, 
	const double s, 
	const double v
) : 
	h(h), 
	s(s), 
	v(v) 
{}

hsv hsv::operator*(const float x) const {
	return hsv(h * x, s * x, v * x);
}

hsv hsv::operator+(const hsv b) const {
	return hsv(h + b.h, s + b.s, v + b.v);
}

void rgba::set(
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

void rgba::set(const rgba col) {
	*this = col;
}

rgba& rgba::multiply_alpha(const float s) {
	a = static_cast<rgba_channel>(s * a);
	return *this;
}

rgba& rgba::multiply_rgb(const float s) {
	r = static_cast<rgba_channel>(s * r);
	g =	static_cast<rgba_channel>(s * g);
	b =	static_cast<rgba_channel>(s * b);

	return *this;
}

rgba rgba::operator*(const rgba s) const {
	return rgba(
		static_cast<rgba_channel>(to_0_1(s.r) * r),
		static_cast<rgba_channel>(to_0_1(s.g) * g),
		static_cast<rgba_channel>(to_0_1(s.b) * b),
		static_cast<rgba_channel>(to_0_1(s.a) * a)
	);
}

rgba rgba::operator*(const float s) const {
	return rgba(
		static_cast<rgba_channel>(s * r),
		static_cast<rgba_channel>(s * g),
		static_cast<rgba_channel>(s * b),
		static_cast<rgba_channel>(s * a)
	);
}

rgba rgba::operator+(const rgba s) const {
	return rgba(
		std::min(255u, static_cast<unsigned>(s.r) + r),
		std::min(255u, static_cast<unsigned>(s.g) + g),
		std::min(255u, static_cast<unsigned>(s.b) + b),
		std::min(255u, static_cast<unsigned>(s.a) + a)
	);
}

rgba rgba::operator-(const rgba s) const {
	return rgba(
		std::max(0, static_cast<int>(r) - static_cast<int>(s.r)),
		std::max(0, static_cast<int>(g) - static_cast<int>(s.g)),
		std::max(0, static_cast<int>(b) - static_cast<int>(s.b)),
		std::max(0, static_cast<int>(a) - static_cast<int>(s.a))
	);
}

rgba& rgba::operator*=(const rgba s) {
	return (*this = *this * s);
}

rgba& rgba::operator+=(const rgba s) {
	return (*this = *this + s);
}

bool rgba::operator==(const rgba v) const {
	return r == v.r && g == v.g && b == v.b && a == v.a;
}

bool rgba::operator!=(const rgba v) const {
	return !operator==(v);
}

hsv rgba::get_hsv() const {
	auto res = rgb2hsv({ r / 255.0, g / 255.0, b / 255.0 });
	return{ res.h / 360.0, res.s, res.v };
}

rgba& rgba::desaturate() {
	const auto avg = static_cast<rgba_channel>((static_cast<unsigned>(r) + g + b) / 3u);
	r = avg;
	g = avg;
	b = avg;
	return *this;
}

rgba_channel& rgba::operator[](const size_t index) {
	return (&r)[index];
}

const rgba_channel& rgba::operator[](const size_t index) const {
	return (&r)[index];
}

rgba::rgb_type& rgba::rgb() {
	return *reinterpret_cast<rgb_type*>(this);
}

const rgba::rgb_type& rgba::rgb() const {
	return *reinterpret_cast<const rgb_type*>(this);
}

rgba& rgba::set_hsv(const hsv hsv) {
	rgb() = hsv;
	return *this;
}

const rgba maroon(0x80, 0x00, 0x00, 0xff);
const rgba red(0xff, 0x00, 0x00, 0xff);
const rgba orange(0xff, 0xA5, 0x00, 0xff);
const rgba yellow(0xff, 0xff, 0x00, 0xff);
const rgba olive(0x80, 0x80, 0x00, 0xff);
const rgba purple(0x80, 0x00, 0x80, 0xff);
const rgba fuchsia(0xff, 0x00, 0xff, 0xff);
const rgba white(0xff, 0xff, 0xff, 0xff);
const rgba lime(0x00, 0xff, 0x00, 0xff);
const rgba green(0, 255, 0, 255); //(0x00, 0x80, 0x00, 0xff);
const rgba navy(0x00, 0x00, 0x80, 0xff);
const rgba blue(0x00, 0x00, 0xff, 0xff);
const rgba aqua(0x00, 0xff, 0xff, 0xff);
const rgba teal(0x00, 0x80, 0x80, 0xff);
const rgba black(0x00, 0x00, 0x00, 0xff);
const rgba silver(0xc0, 0xc0, 0xc0, 0xff);
const rgba gray(0x80, 0x80, 0x80, 0xff);

const rgba ltblue(0, 122, 204, 255);
const rgba dark_green(0, 144, 66, 255);
const rgba pink(255, 0, 255, 255);
const rgba violet(164, 68, 195, 255);
const rgba red_violet(200, 68, 195, 255);
const rgba darkred(122, 0, 0, 255);
const rgba darkgray(30, 30, 30, 255);
const rgba gray1(50, 50, 50, 255);
const rgba gray2(62, 62, 62, 255);
const rgba gray3(104, 104, 104, 255);
const rgba gray4(180, 180, 180, 255);
const rgba slightly_visible_white(255, 255, 255, 15);
const rgba darkblue(6, 5, 20, 255);

const rgba cyan(0, 255, 255, 255);

const rgba vsgreen(87, 166, 74, 255);
const rgba vsdarkgreen(0, 100, 0, 255);
const rgba vscyan(78, 201, 176, 255);
const rgba vsblue(86, 156, 214, 255);

const rgba vsyellow(181, 206, 168, 255);
const rgba vslightgray(220, 220, 220, 255);
const rgba vsdarkgray(127, 127, 127, 255);

const rgba turquoise(0, 146, 222, 255);

std::ostream& operator<<(std::ostream& out, const rgba& x) {
	return x.stream_to(out);
}

std::istream& operator>>(std::istream& in, rgba& x) {
	return x.from_stream(in);
}