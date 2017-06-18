#include "assets_manager.h"
#include "augs/filesystem/file.h"
#include "augs/image/font.h"
#include "augs/window_framework/window.h"
#include "application/content_generation/texture_atlases.h"

#include "game/transcendental/cosmos.h"

#include "augs/templates/introspect.h"
#include "augs/templates/string_to_enum.h"
#include "augs/templates/string_templates.h"
#include "generated/introspectors.h"

void assets_manager::load_baked_metadata(
	const game_image_requests& images,
	const game_font_requests& fonts,
	const atlases_regeneration_output& atlases
) {
	auto& self = *this;

	for (const auto& requested_image : images) {
		const auto image_id = augs::string_to_enum_or<assets::game_image_id>(to_uppercase(requested_image.first));

		ensure(image_id != assets::game_image_id::INVALID);
		// if it is invalid, dynamically allocate id

		auto& baked_image = self[image_id];
		const auto& request = requested_image.second;

		auto assign_atlas_entry = [&](augs::texture_atlas_entry& into, const source_image_path& p) {
			for (const auto& a : atlases.metadatas) {
				const auto it = a.second.images.find(p);

				if (it != a.second.images.end()) {
					const auto found_atlas_entry = (*it).second;
					into = found_atlas_entry;

					break;
				}
			}
		};

		assign_atlas_entry(baked_image.texture_maps[texture_map_type::DIFFUSE], request.source_image_path);

		if (request.neon_map) {
			assign_atlas_entry(baked_image.texture_maps[texture_map_type::NEON], request.get_neon_map_path());
		}

		if (request.generate_desaturation) {
			assign_atlas_entry(baked_image.texture_maps[texture_map_type::DESATURATED], request.get_desaturation_path());
		}

		baked_image.settings = request.gui_usage;

		{
			const auto polygonization_path = request.get_polygonization_path();

			if (augs::file_exists(polygonization_path)) {
				const auto lines = augs::get_file_lines(polygonization_path);

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
			using map_type = std::decay_t<decltype(target_map_of_logical_metas)>;
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