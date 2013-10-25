#pragma once
#include "common.h"

namespace poly_decomposition {
	class Point {
	public:
		Scalar x, y;

		Point();
		Point(Scalar x, Scalar y);

		friend Point operator+(const Point &a, const Point &b);
		friend Scalar area(const Point &a, const Point &b, const Point &c);
		friend bool left(const Point &a, const Point &b, const Point &c);
		friend bool leftOn(const Point &a, const Point &b, const Point &c);
		friend bool right(const Point &a, const Point &b, const Point &c);
		friend bool rightOn(const Point &a, const Point &b, const Point &c);
		friend bool collinear(const Point &a, const Point &b, const Point &c);
		friend Scalar sqdist(const Point &a, const Point &b);
	};
}