#include <Box2D/Box2D.h>

#include "game_image.h"
#include "augs/filesystem/file.h"
#include "application/content_generation/desaturations.h"
#include "application/content_generation/polygonizations_of_images.h"

std::string game_image_definition::get_neon_map_path() const {
	if (custom_neon_map_path) {
		return custom_neon_map_path.value();
	}

	const auto filename = augs::get_filename(source_image_path);

	return augs::replace_filename(source_image_path, "generated/neon_maps/" + filename);
}

std::string game_image_definition::get_desaturation_path() const {
	const auto filename = augs::get_filename(source_image_path);
	return augs::replace_filename(source_image_path, "generated/desaturations/" + filename);
}

std::string game_image_definition::get_scripted_image_path() const {
	return source_image_path;
}

std::string game_image_definition::get_button_with_corners_path_template() const {
	return augs::replace_extension(source_image_path, "_%x.png");
}

std::optional<std::string> game_image_definition::get_polygonization_source_path() const {
	const auto path = augs::replace_extension(source_image_path, "_polygonized.png");

	if (augs::file_exists(path)) {
		return path;
	}

	return std::nullopt;
}

std::optional<std::string> game_image_definition::get_polygonization_output_path() const {
	const auto source_path = get_polygonization_source_path();

	if (source_path) {
		const auto filename = augs::get_filename(source_path.value());
		
		return augs::replace_filename(
			source_path.value(),
			std::string("generated/polygonizations/") + augs::replace_extension(filename, ".points")
		);
	}

	return std::nullopt;
}

std::string game_image_definition::get_default_name() const {
	return augs::get_stem(source_image_path);
}

void game_image_definition::regenerate_resources(const bool force_regenerate) const {
	if (neon_map) {
		ensure(!custom_neon_map_path.has_value() && "neon_map can't be specified if custom_neon_map_path already is.");
		
		regenerate_neon_map(
			source_image_path, 
			get_neon_map_path(), 
			neon_map.value(), 
			force_regenerate
		);
	}
	
	if (generate_desaturation) {
		regenerate_desaturation(
			source_image_path,
			get_desaturation_path(), 
			force_regenerate
		);
	}

	if (scripted_image) {
		regenerate_scripted_image(
			get_scripted_image_path(), 
			scripted_image.value(), 
			force_regenerate
		);
	}

	if (button_with_corners) {
		regenerate_button_with_corners(
			get_button_with_corners_path_template(),
			button_with_corners.value(), 
			force_regenerate
		);
	}

	auto source_path = get_polygonization_source_path();

	const bool found_corresponding_image_for_polygonization = source_path.has_value();

	if (found_corresponding_image_for_polygonization) {
		regenerate_polygonization_of_image(
			source_path.value(),
			get_polygonization_output_path().value(),
			force_regenerate
		);
	}
}

std::vector<source_image_loading_input> game_image_definition::get_atlas_inputs() const {
	decltype(get_atlas_inputs()) output;

	output.push_back({ source_image_path, assets::gl_texture_id::GAME_WORLD_ATLAS });

	if (neon_map) {
		output.push_back({ get_neon_map_path(), assets::gl_texture_id::GAME_WORLD_ATLAS });
	}

	if (generate_desaturation) {
		output.push_back({ get_desaturation_path(), assets::gl_texture_id::GAME_WORLD_ATLAS });
	}

	return output;
}

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