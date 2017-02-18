#include "pixel.h"
#include <limits>
#include "augs/ensure.h"

namespace {
	typedef struct {
		double r;       // percent
		double g;       // percent
		double b;       // percent
	} rgb;
}

static hsv      rgb2hsv(const rgb in);
static rgb      hsv2rgb(const hsv in);

hsv rgb2hsv(const rgb in)
{
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


rgb hsv2rgb(const hsv in)
{
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

rgba::rgba(const console_color c) {
	switch (c) {
	case console_color::WHITE: set(white); break;
	case console_color::RED: set(red); break;
	case console_color::YELLOW: set(yellow); break;
	case console_color::GREEN: set(green); break;
	default: ensure(false); break;
	}
}

rgba::rgba(const rgba_channel red, const rgba_channel green, const rgba_channel blue, const rgba_channel alpha) : r(red), g(green), b(blue), a(alpha) {}

hsv::hsv(const double h, const double s, const double v) : h(h), s(s), v(v) {}

hsv hsv::operator*(const float x) const {
	return hsv(h * x, s * x, v * x);
}

hsv hsv::operator+(const hsv b) const {
	return hsv(h + b.h, s + b.s, v + b.v);
}

void rgba::set(const rgba_channel red, const rgba_channel green, const rgba_channel blue, const rgba_channel alpha) {
	*this = rgba(red, green, blue, alpha);
}

void rgba::set(const rgba col) {
	*this = col;
}

rgba rgba::operator*(const rgba s) const {
	return rgba(
		static_cast<rgba_channel>((s.r / 255.*r / 255.) * 255),
		static_cast<rgba_channel>((s.g / 255.*g / 255.) * 255),
		static_cast<rgba_channel>((s.b / 255.*b / 255.) * 255),
		static_cast<rgba_channel>((s.a / 255.*a / 255.) * 255));
}

rgba rgba::operator*(const float s) const {
	return rgba(
		static_cast<rgba_channel>(s * r),
		static_cast<rgba_channel>(s * g),
		static_cast<rgba_channel>(s * b),
		static_cast<rgba_channel>(s * a));
}

rgba rgba::operator+(const rgba s) const {
	return rgba(
		std::min(255u, static_cast<unsigned>(s.r) + r),
		std::min(255u, static_cast<unsigned>(s.g) + g),
		std::min(255u, static_cast<unsigned>(s.b) + b),
		std::min(255u, static_cast<unsigned>(s.a) + a));
}

rgba rgba::operator-(const rgba s) const {
	return rgba(
		std::max(0, static_cast<int>(r) - static_cast<int>(s.r)),
		std::max(0, static_cast<int>(g) - static_cast<int>(s.g)),
		std::max(0, static_cast<int>(b) - static_cast<int>(s.b)),
		std::max(0, static_cast<int>(a) - static_cast<int>(s.a)));
}

rgba& rgba::operator*=(const rgba b) {
	return (*this = *this * b);
}

bool rgba::operator==(const rgba& v) const {
	return r == v.r && g == v.g && b == v.b && a == v.a;
}

bool rgba::operator!=(const rgba& v) const {
	return !operator==(v);
}

hsv rgba::get_hsv() const {
	auto res = rgb2hsv({ r / 255.0, g / 255.0, b / 255.0 });
	return{ res.h / 360.0, res.s, res.v };
}

rgba rgba::get_desaturated() const {
	const auto avg = static_cast<rgba_channel>((static_cast<unsigned>(r) + g + b) / 3u);

	return { avg, avg, avg, a };
}

rgba::rgb_type& rgba::rgb() {
	return *(rgb_type*)this;
}

const rgba::rgb_type& rgba::rgb() const {
	return *(rgb_type*)this;
}

rgba& rgba::set_hsv(const hsv hsv) {
	const auto res = hsv2rgb({ hsv.h * 360, hsv.s, hsv.v });
	return (*this = rgba{ rgba_channel(res.r * 255), rgba_channel(res.g * 255), rgba_channel(res.b * 255), a });
}

const rgba ltblue(0, 122, 204, 255);
const rgba blue(0, 0, 255, 255);
const rgba red(255, 0, 0, 255);
const rgba dark_green(0, 144, 66, 255);
const rgba green(0, 255, 0, 255);
const rgba orange(255, 165, 0, 255);
const rgba pink(255, 0, 255, 255);
const rgba violet(164, 68, 195, 255);
const rgba darkred(122, 0, 0, 255);
const rgba black(0, 0, 0, 255);
const rgba darkgray(30, 30, 30, 255);
const rgba gray1(50, 50, 50, 255);
const rgba gray2(62, 62, 62, 255);
const rgba gray3(104, 104, 104, 255);
const rgba gray4(180, 180, 180, 255);
const rgba white(255, 255, 255, 255);
const rgba slightly_visible_white(255, 255, 255, 15);
const rgba darkblue(6, 5, 20, 255);

const rgba cyan(0, 255, 255, 255);
const rgba yellow(255, 255, 0, 255);

const rgba vsgreen(87, 166, 74, 255);
const rgba vsdarkgreen(0, 100, 0, 255);
const rgba vscyan(78, 201, 176, 255);
const rgba vsblue(86, 156, 214, 255);

const rgba vsyellow(181, 206, 168, 255);
const rgba vslightgray(220, 220, 220, 255);
const rgba vsdarkgray(127, 127, 127, 255);

const rgba turquoise(0, 146, 222, 255);
