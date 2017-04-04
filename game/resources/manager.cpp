#include "manager.h"
#include <string>
#include "augs/filesystem/file.h"
#include <sstream>
#include "augs/image/font.h"
#include "augs/window_framework/window.h"
#include "game/detail/particle_types.h"
#include "application/content_generation/texture_atlases.h"

using namespace augs;

namespace assets {
	vec2u get_size(game_image_id id) {
		return get_resource_manager().find(id)->texture_maps[texture_map_type::DIFFUSE].original_size_pixels;
	}
}

augs::baked_font& operator*(const assets::font_id& id) {
	return *get_resource_manager().find(id);
}

bool operator!(const assets::font_id& id) {
	return get_resource_manager().find(id) == nullptr;
}

augs::texture_atlas_entry& operator*(const assets::game_image_id& id) {
	return get_resource_manager().find(id)->texture_maps[texture_map_type::DIFFUSE];
}

resources::particle_effect& operator*(const assets::particle_effect_id& id) {
	return *get_resource_manager().find(id);
}

resources::behaviour_tree& operator*(const assets::behaviour_tree_id& id) {
	return *get_resource_manager().find(id);
}

resources::tile_layer& operator*(const assets::tile_layer_id& id) {
	return *get_resource_manager().find(id);
}

augs::sound_buffer& operator*(const assets::sound_buffer_id id) {
	return *get_resource_manager().find(id);
}

bool operator!(const assets::tile_layer_id& id) {
	return get_resource_manager().find(id) == nullptr;
}

bool operator!(const assets::game_image_id& id) {
	return get_resource_manager().find(id) == nullptr;
}

template <class T, class K>
auto ptr_if_found(T& container, const K& id) -> decltype(std::addressof(container[id])) {
	const auto it = container.find(id);

	if (it == container.end()) {
		return nullptr;
	}

	return &(*it).second;
}

namespace resources {
	game_image_baked* manager::find(const assets::game_image_id id) {
		return ptr_if_found(baked_game_images, id);
	}

	game_font_baked* manager::find(const assets::font_id id) {
		return ptr_if_found(baked_game_fonts, id);
	}

	augs::graphics::texture* manager::find(const assets::physical_texture_id id) {
		return ptr_if_found(physical_textures, id);
	}

	graphics::shader_program* manager::find(const assets::program_id id) {
		return ptr_if_found(programs, id);
	}

	behaviour_tree* manager::find(assets::behaviour_tree_id id) {
		return ptr_if_found(behaviour_trees, id);
	}

	particle_effect* manager::find(assets::particle_effect_id id) {
		return ptr_if_found(particle_effects, id);
	}

	tile_layer* manager::find(assets::tile_layer_id id) {
		return ptr_if_found(tile_layers, id);
	}

	augs::sound_buffer* manager::find(const assets::sound_buffer_id id) {
		return ptr_if_found(sound_buffers, id);
	}

	augs::sound_buffer& manager::create(const assets::sound_buffer_id id) {
		augs::sound_buffer& snd = sound_buffers[id];
		return snd;
	}

	void manager::load_baked_metadata(
		const game_image_requests& images,
		const game_font_requests& fonts,
		const atlases_regeneration_output& atlases
	) {
		for (const auto& requested_image : images) {
			auto& baked_image = baked_game_images[requested_image.first];

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
			auto& baked_font = baked_game_fonts[requested_font.first];

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

	void manager::create(
		const assets::physical_texture_id id,
		const bool load_as_binary
	) {
		auto& tex = physical_textures[id];
		
		augs::image atlas_image;

		if (load_as_binary) {
			atlas_image.from_binary_file(typesafe_sprintf("generated/atlases/%x.bin", static_cast<int>(id)));
		}
		else {
			atlas_image.from_file(typesafe_sprintf("generated/atlases/%x.png", static_cast<int>(id)));
		}

		tex.create(atlas_image);
	}

	particle_effect& manager::create(assets::particle_effect_id id) {
		auto& resp = particle_effects[id];
		return resp;
	}

	graphics::shader& manager::create(assets::shader_id id, std::string filename, augs::graphics::shader::type type) {
		graphics::shader& sh = shaders[id];
		sh.create(type, get_file_contents(filename));

		return sh;
	}

	graphics::shader_program& manager::create(assets::program_id program_id, assets::shader_id vertex_id, assets::shader_id fragment_id) {
		graphics::shader_program& p = programs[program_id];
		p.create(); 
		p.attach(shaders[vertex_id]);
		p.attach(shaders[fragment_id]);
		p.build();

		return p;
	}

	behaviour_tree& manager::create(assets::behaviour_tree_id id) {
		auto& tree = behaviour_trees[id];
		return tree;
	}

	tile_layer& manager::create(assets::tile_layer_id id) {
		auto& layer = tile_layers[id];
		return layer;
	}

	void manager::destroy_everything() {
		this->~manager();
		new (this) manager;
	}
}