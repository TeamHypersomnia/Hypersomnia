#pragma once
#include "augs/math/rects.h"

namespace augs {
	template <class T, class A, class F>
	void general_border(
		basic_ltrb<T> bordered,
	   	const A total_expansion,
		const A width,
	   	F&& callback
	) {
		if (total_expansion) {
			bordered.l -= total_expansion;
			bordered.t -= total_expansion;
			bordered.r += total_expansion;
			bordered.b += total_expansion;
		}

		basic_ltrb<T> lines[4] = {
			bordered,
			bordered,
			bordered,
			bordered
		};

		/*
			Side borders reach the corners,
			the bottom and the top borders are in-between them.
		*/

		/* Left border */
		lines[0].r = bordered.l + width;

		/* Top border */
		lines[1].b = bordered.t + width;
		lines[1].r -= width;
		lines[1].l += width;

		/* Right border */
		lines[2].l = bordered.r - width;

		/* Bottom border */
		lines[3].t = bordered.b - width;
		lines[3].r -= width;
		lines[3].l += width;

		for (const auto& l : lines) {
			callback(l);
		}
	}

	template <class T, class A, class F>
	void general_border_from_to(
		basic_ltrb<T> bordered,
		const A total_expansion,
		F&& callback
	) {
		if (total_expansion) {
			bordered.l -= total_expansion;
			bordered.t -= total_expansion;
			bordered.r += total_expansion;
			bordered.b += total_expansion;
		}

		using v = basic_vec2<T>;

		struct fromto {
			v from;
			v to;
		};

		fromto lines[4] = {
			{ bordered.left_top(), bordered.right_top() },
			{ bordered.left_bottom(), bordered.right_bottom() },
			{ bordered.left_top() + v(0, 1), bordered.left_bottom() - v(0, 1) },
			{ bordered.right_top() + v(0, 1), bordered.right_bottom() - v(0, 1) }
		};

		for (const auto& l : lines) {
			callback(l.from, l.to);
		}
	}
}
