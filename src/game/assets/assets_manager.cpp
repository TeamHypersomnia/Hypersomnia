#include "assets_manager.h"
#include "augs/filesystem/file.h"
#include "augs/image/font.h"
#include "augs/window_framework/window.h"
#include "application/content_generation/texture_atlases.h"

#include "game/transcendental/cosmos.h"

#include "augs/templates/introspect.h"
#include "generated/introspectors.h"

void assets_manager::load_baked_metadata(
	const game_image_requests& images,
	const game_font_requests& fonts,
	const atlases_regeneration_output& atlases
) {
	auto& self = *this;

	for (const auto& requested_image : images) {
		auto& baked_image = self[requested_image.first];

		for (size_t i = 0; i < baked_image.texture_maps.size(); ++i) {
			const auto seeked_identifier = requested_image.second.texture_maps[i].path;
			const bool this_texture_map_is_not_used = seeked_identifier.empty();

			if (this_texture_map_is_not_used) {
				continue;
			}

			for (const auto& a : atlases.metadatas) {
				const auto it = a.second.images.find(seeked_identifier);

				if (it != a.second.images.end()) {
					const auto found_atlas_entry = (*it).second;
					baked_image.texture_maps[i] = found_atlas_entry;

					break;
				}
			}
		}

		baked_image.settings = requested_image.second.settings;

		{
			if (requested_image.second.polygonization_filename.size() > 0) {
				const auto lines = augs::get_file_lines(requested_image.second.polygonization_filename);

				for (const auto& l : lines) {
					std::istringstream in(l);

					vec2u new_point;
					in >> new_point;

					baked_image.polygonized.push_back(new_point);
				}
			}
		}
	}

	for (const auto& requested_font : fonts) {
		auto& baked_font = self[requested_font.first];

		const auto seeked_identifier = requested_font.second.loading_input;

		ensure(seeked_identifier.path.size() > 0);
		ensure(seeked_identifier.characters.size() > 0);
		ensure(seeked_identifier.pt > 0);

		for (const auto& a : atlases.metadatas) {
			const auto it = a.second.fonts.find(seeked_identifier);

			if (it != a.second.fonts.end()) {
				const auto found_atlas_entry = (*it).second;
				baked_font = found_atlas_entry;

				break;
			}
		}
	}
}

void assets_manager::create(
	const assets::gl_texture_id id,
	const bool load_as_binary
) {
	auto& self = *this;
	auto& tex = self[id];

	augs::image atlas_image;

	if (load_as_binary) {
		atlas_image.from_binary_file(typesafe_sprintf("generated/atlases/%x.bin", static_cast<int>(id)));
	}
	else {
		atlas_image.from_file(typesafe_sprintf("generated/atlases/%x.png", static_cast<int>(id)));
	}

	tex.create(atlas_image);
}

augs::graphics::shader_program& assets_manager::create(
	const assets::program_id program_id, 
	const assets::shader_id vertex_id, 
	const assets::shader_id fragment_id
) {
	auto& self = *this;

	auto& p = self[program_id];
	p.create();
	p.attach(self[vertex_id]);
	p.attach(self[fragment_id]);
	p.build();

	return p;
}

all_logical_metas_of_assets assets_manager::generate_logical_metas_of_assets() const {
	all_logical_metas_of_assets output;

	augs::introspect(
		[this](auto, auto& target_map_of_logical_metas) {
			typedef std::decay_t<decltype(target_map_of_logical_metas)> map_type;
			target_map_of_logical_metas.clear();
	
			const auto& source_map_of_assets = 
				get_container_with_key_type<typename map_type::key_type>(all)
			;
	
			for (const auto& asset_entry : source_map_of_assets) {
				target_map_of_logical_metas[asset_entry.first] = asset_entry.second.get_logical_meta(*this);
			}
		},
		output.all
	);

	return output;
}

void assets_manager::destroy_everything() {
	this->~assets_manager();
	new (this) assets_manager;
}

#if ONLY_ONE_GLOBAL_ASSETS_MANAGER
assets_manager assets_manager::global_instance;
#endif