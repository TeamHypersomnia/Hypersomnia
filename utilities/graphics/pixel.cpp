#include "pixel.h"
#include <limits>

namespace augs {
	namespace graphics {
		typedef struct {
			double r;       // percent
			double g;       // percent
			double b;       // percent
		} rgb;

		static hsv      rgb2hsv(rgb in);
		static rgb      hsv2rgb(hsv in);

		hsv rgb2hsv(rgb in)
		{
			hsv         out;
			double      min, max, delta;

			min = in.r < in.g ? in.r : in.g;
			min = min  < in.b ? min : in.b;

			max = in.r > in.g ? in.r : in.g;
			max = max  > in.b ? max : in.b;

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


		rgb hsv2rgb(hsv in)
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

		pixel_8::pixel_8(color brightness) : r(brightness) {}
		pixel_8::operator unsigned char() {return r;}
		pixel_8& pixel_8::operator=(color brightness) { r = brightness; return *this; } 
		
		pixel_24::pixel_24(color red, color green, color blue) : r(red), g(green), b(blue) {}
		pixel_32::pixel_32(color red, color green, color blue, color alpha) : r(red), g(green), b(blue), a(alpha) {}

		hsv::hsv(double h, double s, double v) : h(h), s(s), v(v) {}

		hsv pixel_32::get_hsv() const {
			auto res = rgb2hsv({ r /255.0, g / 255.0, b / 255.0 });
			return { res.h / 360.0, res.s, res.v };
		}

		pixel_32& pixel_32::set_hsv(hsv hsv) {
			auto res = hsv2rgb({ hsv.h * 360, hsv.s, hsv.v });
			return (*this = pixel_32{ color(res.r * 255), color(res.g * 255), color(res.b * 255), a });
		}

		namespace colors {
			const pixel_32 ltblue(0, 122, 204, 255);
			const pixel_32 blue(0, 61, 102, 255);
			const pixel_32 red(255, 0, 0, 255);
			const pixel_32 green(0, 144, 66, 255);
			const pixel_32 violet(164, 68, 195, 255);
			const pixel_32 darkred(122, 0, 0, 255);
			const pixel_32 black(0, 0, 0, 255);
			const pixel_32 darkgray(30, 30, 30, 255);
			const pixel_32 gray1(50, 50, 50, 255);
			const pixel_32 gray2(62, 62, 62, 255);
			const pixel_32 gray3(104, 104, 104, 255);
			const pixel_32 gray4(180, 180, 180, 255);
			const pixel_32 white(255, 255, 255, 255);
			const pixel_32 darkblue(6, 5, 20, 255);
		}
	}
}
