#pragma once
#include "pixel.h"

namespace augmentations {
	namespace graphics {
		pixel_8::pixel_8(color brightness) : r(brightness) {}
		pixel_8::operator unsigned char() {return r;}
		pixel_8& pixel_8::operator=(color brightness) { r = brightness; return *this; } 
		
		pixel_24::pixel_24(color red, color green, color blue) : r(red), g(green), b(blue) {}
		pixel_32::pixel_32(color red, color green, color blue, color alpha) : r(red), g(green), b(blue), a(alpha) {}

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
