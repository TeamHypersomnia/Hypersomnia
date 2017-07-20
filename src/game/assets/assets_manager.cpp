#include "assets_manager.h"
#include "augs/filesystem/file.h"
#include "augs/image/font.h"
#include "augs/window_framework/window.h"
#include "application/content_regeneration/texture_atlases.h"

#include "game/transcendental/cosmos.h"

#include "augs/templates/introspect.h"
#include "augs/templates/string_to_enum.h"
#include "augs/templates/string_templates.h"
#include "generated/introspectors.h"

void assets_manager::load_baked_metadata(
	const game_image_definitions& images,
	const game_font_definitions& fonts,
	const atlases_regeneration_output& atlases
) {
	auto& self = *this;

	auto is_reserved_path = [](auto p) {
		const std::string requisite_dir[] = {
			"content\\requisite\\",
			"generated\\content\\requisite\\",
			"content/requisite/",
			"generated/content/requisite/",
			"content\\official\\",
			"generated\\content\\official\\",
			"content/official/",
			"generated/content/official/"
		};
		
		for (auto dir : requisite_dir) {
			if (dir == p.substr(0, dir.length())) {
				return true;
			}
		}

		return false;
	};

	for (const auto& requested_image : images) {
		const auto definition_path = requested_image.first;
		const auto& request = requested_image.second;

		const bool is_reserved_asset = is_reserved_path(definition_path);
		const auto diffuse_map_path = request.get_source_image_path(definition_path);

		assets::game_image_id image_id = assets::game_image_id::INVALID;

		if (is_reserved_asset) {
			image_id = augs::string_to_enum_or<assets::game_image_id>(to_uppercase(augs::get_stem(diffuse_map_path)));
		}
		else {
			// if it is not reserved, dynamically allocate id
		}

		ensure(is_reserved_asset);
		ensure(image_id != assets::game_image_id::INVALID);

		auto& baked_image = self[image_id];

		auto assign_atlas_entry = [&](
			augs::texture_atlas_entry& into, 
			const source_image_identifier& p
		) {
			for (const auto& a : atlases.metadatas) {
				const auto it = a.second.images.find(p);

				if (it != a.second.images.end()) {
					const auto found_atlas_entry = (*it).second;
					into = found_atlas_entry;

					break;
				}
			}
		};

		assign_atlas_entry(baked_image.texture_maps[texture_map_type::DIFFUSE], diffuse_map_path);

		if (request.has_neon_map()) {
			assign_atlas_entry(baked_image.texture_maps[texture_map_type::NEON], request.get_neon_map_path(definition_path));
		}

		if (request.generate_desaturation) {
			assign_atlas_entry(baked_image.texture_maps[texture_map_type::DESATURATED], request.get_desaturation_path(definition_path));
		}

		baked_image.gui_usage = request.gui_usage;

		if (request.physical_shape.has_value()) {
			baked_image.polygonized = request.physical_shape.value();
		}
	}

	for (const auto& requested_font : fonts) {
		const auto definition_path = requested_font.first;

		const bool is_reserved_asset = is_reserved_path(definition_path);

		assets::font_id font_id = assets::font_id::INVALID;

		if (is_reserved_asset) {
			font_id = augs::string_to_enum_or<assets::font_id>(to_uppercase(augs::get_stem(definition_path)));
		}
		else {
			// if it is not reserved, dynamically allocate id
		}

		ensure(is_reserved_asset);

		// if it is not reserved, dynamically allocate id
		if (!is_reserved_asset) {

		}

		auto& baked_font = self[font_id];

		const auto seeked_identifier = requested_font.second.loading_input;

		ensure(seeked_identifier.path.size() > 0);
		ensure(seeked_identifier.input.charset_path.size() > 0);
		ensure(seeked_identifier.input.pt > 0);

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

void assets_manager::load_requisite(const assets::gl_texture_id id) {
	augs::enum_associative_array<assets::gl_texture_id, augs::graphics::texture> ch;
	augs::constant_size_vector< augs::graphics::texture, 3> aaaaa;

	get_store_by<assets::gl_texture_id>().emplace(
		id,
		augs::image(
			typesafe_sprintf(
				"generated/atlases/%x",
				augs::enum_to_string(id)
			)
		)
	);
}

std::unique_ptr<all_logical_metas_of_assets> assets_manager::generate_logical_metas_of_assets() const {
	auto output = std::make_unique<all_logical_metas_of_assets>();

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
		output->all
	);

	return std::move(output);
}