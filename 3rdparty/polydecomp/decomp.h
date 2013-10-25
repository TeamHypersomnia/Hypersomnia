#include "Point.h"

namespace poly_decomposition {
	typedef std::vector<Point> Polygon;

	void makeCCW(Polygon &poly);
	std::vector<Polygon> decompose_polygon(Polygon poly);
}