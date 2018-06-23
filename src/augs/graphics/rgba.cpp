#include <limits>
#include <array>
#include <algorithm>
#include <imgui/imgui.h>
#include "augs/graphics/rgba.h"

namespace rgba_detail {
	hsv rgb2hsv(const rgb in) {
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

	rgb hsv2rgb(const hsv in) {
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
}

std::ostream& operator<<(std::ostream& out, const rgba& x) {
	return x.stream_to(out);
}

std::istream& operator>>(std::istream& in, rgba& x) {
	return x.from_stream(in);
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

#include "augs/log.h"

void rgba::avoid_dark_blue_for_color_wave() {
	if (b == 255) {
		auto& f1 = g;
		auto& f2 = r;
		const auto threshold = 100;

		if (f1 == 0) {
			f2 = std::max(threshold, int(f2));
		}
		else {
			if (f1 < threshold) {
				f2 = threshold - f1 / 3;
			}
		}
	}
}

