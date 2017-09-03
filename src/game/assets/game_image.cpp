#include <Box2D/Box2D.h>

#include "augs/image/image.h"
#include "augs/filesystem/file.h"

#include "game/assets/game_image.h"

#include "application/content_regeneration/desaturations.h"

#include "generated/introspectors.h"

augs::path_type get_neon_map_path(augs::path_type from_source_path) {
	return augs::path_type("generated/") += from_source_path.replace_extension(".neon_map.png");
}

augs::path_type get_desaturation_path(augs::path_type from_source_path) {
	return augs::path_type("generated/") += from_source_path.replace_extension(".desaturation.png");
}

augs::path_type game_image_definition::get_source_image_path() const {
	return source_image_path;
}

std::optional<augs::path_type> game_image_definition::get_neon_map_path() const {
	if (custom_neon_map_path) {
		return get_source_image_path().replace_filename(*custom_neon_map_path);
	}
	else if (neon_map) {
		return ::get_neon_map_path(source_image_path);
	}

	return std::nullopt;
}

std::optional<augs::path_type> game_image_definition::get_desaturation_path() const {
	if (should_generate_desaturation()) {
		return ::get_desaturation_path(source_image_path);
	}

	return std::nullopt;
}

vec2u game_image_definition::get_size() const {
	return augs::image::get_size(get_source_image_path());
}

void game_image_definition::regenerate_all_needed(
	const bool force_regenerate
) const {
	const auto diffuse_path = get_source_image_path();

	if (neon_map) {
		ensure(!custom_neon_map_path.has_value() && "neon_map can't be specified if custom_neon_map_path already is.");
		
		regenerate_neon_map(
			diffuse_path,
			get_neon_map_path().value(),
			neon_map.value(), 
			force_regenerate
		);
	}
	
	if (should_generate_desaturation()) {
		regenerate_desaturation(
			diffuse_path,
			get_desaturation_path().value(),
			force_regenerate
		);
	}
}

game_image_logical::game_image_logical(const game_image_definition& source) {
	original_image_size = source.get_size();

	if (source.physical_shape.has_value()) {
		const auto image_size = vec2(get_size());

		std::vector<vec2> new_concave;

		for (vec2 v : source.physical_shape.value()) {
			v.y = -v.y;
			new_concave.push_back(v);
		}

		const auto origin = image_size / vec2(-2, 2);

		for (auto& v : new_concave) {
			v += origin;
		}

		shape.add_concave_polygon(new_concave);
		shape.scale(vec2(1, -1));

		for (auto& c : shape.convex_polys) {
			reverse_container(c);
		}
	}
	else {
		const auto box_size = vec2(get_size()) / 2;
		
		// TODO: use ltrb

		b2PolygonShape poly_shape;
		poly_shape.SetAsBox(box_size.x, box_size.y);

		convex_partitioned_shape::convex_poly new_convex_polygon;

		for (int i = 0; i < poly_shape.GetVertexCount(); ++i) {
			new_convex_polygon.push_back(vec2(poly_shape.GetVertex(i)));
		}

		shape.add_convex_polygon(new_convex_polygon);
	}
}