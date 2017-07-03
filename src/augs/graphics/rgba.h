#pragma once
#include <iosfwd>
#include "augs/console_color.h"

typedef unsigned char rgba_channel;

struct hsv {
	double h;       // angle in degrees
	double s;       // percent
	double v;       // percent
	hsv(double = 0.0, double = 0.0, double = 0.0);
	hsv operator*(float) const;
	hsv operator+(hsv b) const;
};

inline auto to_0_1(const rgba_channel c) {
	return c / 255.f;
}

inline auto to_0_255(const float c) {
	return static_cast<rgba_channel>(c * 255.f);
}

struct ImVec4;

struct rgba {
	struct rgb_type {
		rgba_channel r;
		rgba_channel g;
		rgba_channel b;
	};

	// GEN INTROSPECTOR struct rgba
	rgba_channel r;
	rgba_channel g;
	rgba_channel b;
	rgba_channel a;
	// END GEN INTROSPECTOR

	explicit rgba(const console_color);
	
	rgba(const ImVec4&);
	operator ImVec4() const;

	rgba(
		const rgb_type, 
		const rgba_channel alpha = 255
	);

	rgba(
		const rgba_channel red = 255, 
		const rgba_channel green = 255, 
		const rgba_channel blue = 255, 
		const rgba_channel alpha = 255
	);

	void set(
		const rgba_channel red = 255, 
		const rgba_channel green = 255, 
		const rgba_channel blue = 255, 
		const rgba_channel alpha = 255
	);

	void set(const rgba);

	rgba operator*(const float) const;
	rgba operator+(const rgba b) const;
	rgba operator-(const rgba b) const;
	rgba operator*(const rgba b) const;
	rgba& operator*=(const rgba b);
	rgba& operator+=(const rgba b);

	bool operator==(const rgba b) const;
	bool operator!=(const rgba b) const;
	hsv get_hsv() const;
	rgba get_desaturated() const;

	rgba_channel& operator[](const size_t index);
	const rgba_channel& operator[](const size_t index) const;

	rgb_type& rgb();
	const rgb_type& rgb() const;

	rgba& set_hsv(const hsv);

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

extern const rgba ltblue;
extern const rgba blue;
extern const rgba red;
extern const rgba dark_green;
extern const rgba green;
extern const rgba orange;
extern const rgba violet;
extern const rgba red_violet;
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

extern const rgba turquoise;
