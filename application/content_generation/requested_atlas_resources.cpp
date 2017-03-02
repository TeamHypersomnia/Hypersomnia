#include "atlas_content_structs.h"

#include <experimental\filesystem>

namespace fs = std::experimental::filesystem;

void requested_atlas_resources::request(
	const assets::texture_id id,
	const std::string& diffuse_filename
) {
	auto& r = images[id];

	r.textures[image_map_type::DIFFUSE] = diffuse_filename;

	const auto neon_map_filename = "generated/neon_maps/" + fs::path(diffuse_filename).filename().string();
	const auto desaturation_map_filename = "generated/desaturations/" + fs::path(diffuse_filename).filename().string();

	if (fs::exists(neon_map_filename)) {
		r.textures[image_map_type::NEON] = neon_map_filename;
	}

	if (fs::exists(desaturation_map_filename)) {
		r.textures[image_map_type::DESATURATED] = neon_map_filename;
	}
}

void requested_atlas_resources::request(
	const assets::font_id id,
	const augs::font_loading_input in
) {
	fonts[id] = in;
}

void requested_atlas_resources::request_indexed(
	const assets::texture_id first,
	const assets::texture_id last,
	const std::string& diffuse_filename
) {

}

void requested_atlas_resources::request_button_with_corners(
	const assets::texture_id inside_image_id,
	const std::string& first_filename
) {

}
