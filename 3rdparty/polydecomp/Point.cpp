#include "Point.h"

namespace poly_decomposition {
	Point::Point() : x(0), y(0) {
	}

	Point::Point(Scalar x, Scalar y) : x(x), y(y) {
	}

	Point operator+(const Point &a, const Point &b) {
		return Point(a.x + b.x, b.y + b.y);
	}

	Scalar area(const Point &a, const Point &b, const Point &c) {
		return (((b.x - a.x)*(c.y - a.y)) - ((c.x - a.x)*(b.y - a.y)));
	}

	bool left(const Point &a, const Point &b, const Point &c) {
		return area(a, b, c) > 0;
	}

	bool leftOn(const Point &a, const Point &b, const Point &c) {
		return area(a, b, c) >= 0;
	}

	bool right(const Point &a, const Point &b, const Point &c) {
		return area(a, b, c) < 0;
	}

	bool rightOn(const Point &a, const Point &b, const Point &c) {
		return area(a, b, c) <= 0;
	}

	bool collinear(const Point &a, const Point &b, const Point &c) {
		return area(a, b, c) == 0;
	}

	Scalar sqdist(const Point &a, const Point &b) {
		Scalar dx = b.x - a.x;
		Scalar dy = b.y - a.y;
		return dx * dx + dy * dy;
	}
}
