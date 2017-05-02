#include "game_image.h"

#include <Box2D/Box2D.h>

game_image_logical_meta game_image_baked::get_logical_meta(const assets_manager& manager) const {
	game_image_logical_meta output;

	auto& shape = output.shape;

	if (polygonized.size() > 0) {
		const auto image_size = get_size();

		std::vector<vec2> new_concave;

		for (auto v : polygonized) {
			vec2 new_v = v;

			new_v.y = -new_v.y;
			new_concave.push_back(new_v);
		}

		const vec2 origin = vec2(static_cast<float>(image_size.x) / -2.f, image_size.y / 2.f);

		for (auto& v : new_concave) {
			v += origin;
		}

		shape.add_concave_polygon(new_concave);
		shape.scale(vec2(1, -1));

		for (auto& c : shape.convex_polys) {
			std::reverse(c.vertices.begin(), c.vertices.end());
		}
	}
	else {
		const auto rect_size = get_size();

		b2PolygonShape poly_shape;
		poly_shape.SetAsBox(static_cast<float>(rect_size.x) / 2.f, static_cast<float>(rect_size.y) / 2.f);

		convex_poly new_convex_polygon;

		for (int i = 0; i < poly_shape.GetVertexCount(); ++i) {
			new_convex_polygon.vertices.push_back(vec2(poly_shape.GetVertex(i)));
		}

		shape.add_convex_polygon(new_convex_polygon);
	}

	output.original_image_size = get_size();
	return output;
}