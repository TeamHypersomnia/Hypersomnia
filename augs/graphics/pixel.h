#pragma once
#include <array>
namespace augs {
	typedef unsigned char rgba_channel;

	struct hsv {
		double h;       // angle in degrees
		double s;       // percent
		double v;       // percent
		hsv(double = 0.0, double = 0.0, double = 0.0);
	};

	struct rgba {
		rgba_channel r, g, b, a;
		rgba(rgba_channel red = 255, rgba_channel green = 255, rgba_channel blue = 255, rgba_channel alpha = 255);
		void set(rgba_channel red = 255, rgba_channel green = 255, rgba_channel blue = 255, rgba_channel alpha = 255);

		rgba operator*(rgba b) const;
		rgba& operator*=(rgba b);

		bool operator==(const rgba& b) const;
		hsv get_hsv() const;
		std::array<rgba_channel, 3>& rgb();
		const std::array<rgba_channel, 3>& rgb() const;
		rgba& set_hsv(hsv);
	};

	extern const rgba ltblue;
	extern const rgba blue;
	extern const rgba red;
	extern const rgba dark_green;
	extern const rgba green;
	extern const rgba orange;
	extern const rgba violet;
	extern const rgba pink;
	extern const rgba darkred;
	extern const rgba black;
	extern const rgba darkgray;
	extern const rgba gray1;
	extern const rgba gray2;
	extern const rgba gray3;
	extern const rgba gray4;
	extern const rgba slightly_visible_white;
	extern const rgba white;
	extern const rgba darkblue;

	extern const rgba cyan;
	extern const rgba yellow;

	extern const rgba vsblue;
	extern const rgba vscyan;

	extern const rgba vsgreen;
	extern const rgba vsdarkgreen;

	extern const rgba vsyellow;
	extern const rgba vslightgray;
	extern const rgba vsdarkgray;
}