#include <Box2D/Box2D.h>

#include "game_image.h"
#include "augs/filesystem/file.h"
#include "application/content_generation/desaturations.h"
#include "augs/gui/button_corners_type.h"
#include "augs/gui/button_corners.h"

#include "generated/introspectors.h"

std::string game_image_definition::get_source_image_path(const std::string& from_definition_path) const {
	if (scripted_image.has_value()) {
		return get_scripted_image_path(from_definition_path);
	}
	
	return augs::replace_extension(from_definition_path, ".png");
}

std::vector<std::string> game_image_definition::get_diffuse_map_paths(const std::string& from_definition_path) const {
	if (button_with_corners.has_value()) {
		return get_button_with_corners_paths(from_definition_path);
	}

	return { get_source_image_path(from_definition_path) };
}

std::string game_image_definition::get_neon_map_path(const std::string& from_definition_path) const {
	if (custom_neon_map_path) {
		return custom_neon_map_path.value();
	}

	const auto filename = augs::replace_extension(augs::get_filename(from_definition_path), ".png");
	return augs::replace_filename("generated/" + from_definition_path, "neon_maps/" + filename);
}

std::string game_image_definition::get_desaturation_path(const std::string& from_definition_path) const {
	const auto filename = augs::replace_extension(augs::get_filename(from_definition_path), ".png");
	return augs::replace_filename("generated/" + from_definition_path, "desaturations/" + filename);
}

std::string game_image_definition::get_scripted_image_path(const std::string& from_definition_path) const {
	const auto filename = augs::replace_extension(augs::get_filename(from_definition_path), ".png");
	return augs::replace_filename("generated/" + from_definition_path, "scripted/" + filename);
}

std::string game_image_definition::get_button_with_corners_path_template(const std::string& from_definition_path) const {
	const auto filename = augs::get_stem(from_definition_path) + "_%x.png";
	return augs::replace_filename("generated/" + from_definition_path, "buttons_with_corners/" + filename);
}

std::vector<std::string> game_image_definition::get_button_with_corners_paths(const std::string& from_definition_path) const {
	const auto path_template = get_button_with_corners_path_template(from_definition_path);
	
	std::vector<std::string> out;

	augs::for_each_enum<button_corner_type>(
		[&out, path_template, this](const auto e){ 
			if (e == button_corner_type::COUNT) {
				return;
			}

			if (!button_with_corners.value().make_lb_complement && is_lb_complement(e)) {
				return;
			}

			out.push_back(typesafe_sprintf(path_template, to_lowercase(std::string(augs::enum_to_string(e)))));
		}
	);

	return out;
}

void game_image_definition::regenerate_resources(
	const std::string& definition_path,
	const bool force_regenerate
) const {
	if (scripted_image) {
		regenerate_scripted_image(
			get_scripted_image_path(definition_path),
			scripted_image.value(),
			force_regenerate
		);
	}

	if (button_with_corners) {
		regenerate_button_with_corners(
			get_button_with_corners_path_template(definition_path),
			button_with_corners.value(),
			force_regenerate
		);
	}

	for (const auto& diffuse_path : get_diffuse_map_paths(definition_path)) {
		if (neon_map) {
			ensure(!custom_neon_map_path.has_value() && "neon_map can't be specified if custom_neon_map_path already is.");
			
			regenerate_neon_map(
				diffuse_path,
				get_neon_map_path(diffuse_path),
				neon_map.value(), 
				force_regenerate
			);
		}
		
		if (should_generate_desaturation()) {
			regenerate_desaturation(
				diffuse_path,
				get_desaturation_path(diffuse_path),
				force_regenerate
			);
		}
	}
}

std::vector<source_image_loading_input> game_image_definition::get_atlas_inputs(const std::string& definition_path) const {
	decltype(get_atlas_inputs(definition_path)) output;

	for (const auto p : get_diffuse_map_paths(definition_path)) {
		output.push_back({ p, assets::gl_texture_id::GAME_WORLD_ATLAS });
	}

	if (neon_map) {
		output.push_back({ get_neon_map_path(definition_path), assets::gl_texture_id::GAME_WORLD_ATLAS });
	}

	if (should_generate_desaturation()) {
		output.push_back({ get_desaturation_path(definition_path), assets::gl_texture_id::GAME_WORLD_ATLAS });
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