#pragma once
namespace augs {
	namespace graphics {
		typedef unsigned char color;

		struct pixel_8 {
			color r;
			pixel_8(color brightness = 255);
			operator color ();
			pixel_8& operator=(color);
		};

		struct pixel_24 {
			color r, g, b;
			pixel_24(color red = 255, color green = 255, color blue = 255);
		};

		struct pixel_32 {
			color r, g, b, a;
			pixel_32(color red = 255, color green = 255, color blue = 255, color alpha = 255);
		};

		namespace colors {
			extern const pixel_32 ltblue;
			extern const pixel_32 blue;
			extern const pixel_32 red;
			extern const pixel_32 green;
			extern const pixel_32 violet;
			extern const pixel_32 darkred;
			extern const pixel_32 black;
			extern const pixel_32 darkgray;
			extern const pixel_32 gray1;
			extern const pixel_32 gray2;
			extern const pixel_32 gray3;
			extern const pixel_32 gray4;
			extern const pixel_32 white;
			extern const pixel_32 darkblue;
		}
	}
}