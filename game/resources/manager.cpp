#include "manager.h"
#include <string>
#include "augs/filesystem/file.h"
#include <sstream>
#include "augs/texture_baker/font.h"

using namespace augs;

namespace assets {
	vec2i get_size(texture_id id) {
		return (*id).get_size();
	}
}

augs::font& operator*(const assets::font_id& id) {
	return *resource_manager.find(id);
}

bool operator!(const assets::font_id& id) {
	return resource_manager.find(id) == nullptr;
}

augs::texture& operator*(const assets::texture_id& id) {
	return resource_manager.find(id)->tex;
}

resources::animation_response& operator*(const assets::animation_response_id& id) {
	return *resource_manager.find(id);
}

resources::particle_effect_response& operator*(const assets::particle_effect_response_id& id) {
	return *resource_manager.find(id);
}

resources::particle_effect& operator*(const assets::particle_effect_id& id) {
	return *resource_manager.find(id);
}

resources::behaviour_tree& operator*(const assets::behaviour_tree_id& id) {
	return *resource_manager.find(id);
}

bool operator!(const assets::texture_id& id) {
	return resource_manager.find(id) == nullptr;
}

namespace resources {
	texture_with_image* manager::find(const assets::texture_id id) {
		auto it = textures.find(id);
		if (it == textures.end()) return nullptr;

		return &(*it).second;
	}	

	texture_with_image* manager::find_neon_map(const assets::texture_id id) {
		auto it = neon_maps.find(id);
		
		if (it == neon_maps.end()) {
			return nullptr;
		}

		return &(*it).second;
	}

	atlas* manager::find(assets::atlas_id id) {
		auto it = atlases.find(id);
		if (it == atlases.end()) return nullptr;

		return &(*it).second;
	}

	font* manager::find(assets::font_id id) {
		auto it = fonts.find(id);
		if (it == fonts.end()) return nullptr;

		return &(*it).second;
	}

	graphics::shader_program* manager::find(assets::program_id id) {
		auto it = programs.find(id);
		if (it == programs.end()) return nullptr;

		return &(*it).second;
	}

	animation* manager::find(assets::animation_id id) {
		auto it = animations.find(id);
		if (it == animations.end()) return nullptr;

		return &(*it).second;
	}

	animation_response* manager::find(assets::animation_response_id id) {
		auto it = animation_responses.find(id);
		if (it == animation_responses.end()) return nullptr;

		return &(*it).second;
	}

	particle_effect_response* manager::find(assets::particle_effect_response_id id) {
		auto it = particle_effect_responses.find(id);
		if (it == particle_effect_responses.end()) return nullptr;

		return &(*it).second;
	}

	behaviour_tree* manager::find(assets::behaviour_tree_id id) {
		auto it = behaviour_trees.find(id);
		if (it == behaviour_trees.end()) return nullptr;

		return &(*it).second;
	}

	particle_effect* manager::find(assets::particle_effect_id id) {
		auto it = particle_effects.find(id);
		if (it == particle_effects.end()) return nullptr;

		return &(*it).second;
	}

	atlas& manager::create(const assets::atlas_id id, const unsigned atlas_creation_mode_flags) {
		atlas& atl = atlases[id];

		if (atlas_creation_mode_flags & atlas_creation_mode::FROM_ALL_TEXTURES) {
			for (const auto& tex : textures) {
				atl.textures.push_back(&tex.second);
			}			
			
			for (const auto& tex : neon_maps) {
				atl.textures.push_back(&tex.second);
			}
		}

		if (atlas_creation_mode_flags & atlas_creation_mode::FROM_ALL_FONTS) {
			for (const auto& fnt : fonts) {
				fnt.second.add_to_atlas(atl);
			}
		}

		atl.default_build();

		return atl;
	}

	augs::font& manager::create(const assets::font_id id) {
		augs::font& font = fonts[id];

		return font;
	}

	texture_with_image& manager::create(const assets::texture_id id, const image img) {
		texture_with_image& tex = textures[id];
		tex.set_from_image(img);

		return tex;
	}

	void manager::create_sprites_indexed(assets::texture_id first, assets::texture_id last, std::string filename_preffix) {
		for (assets::texture_id i = first; i < last; i = assets::texture_id(int(i) + 1)) {
			std::ostringstream filename;
			filename << filename_preffix << "_" << int(int(i) - int(first) + 1) << ".png";

			create(i, filename.str());
		}
	}

	animation& manager::create(assets::animation_id id) {
		animation& anim = animations[id];
		return anim;
	}

	animation& manager::create(assets::animation_id id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms, 
		resources::animation::loop_type loop_mode) {
		
		animation& anim = create(id);
		anim.loop_mode = loop_mode;

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(int(i)+1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		return anim;
	}
	
	animation& manager::create_inverse(assets::animation_id id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms) {
		animation& anim = create(id);
		anim.loop_mode = animation::loop_type::INVERSE;

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(int(i) + 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		return anim;
	}

	animation& manager::create_inverse_with_flip(assets::animation_id id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms) {
		animation& anim = create(id);
		anim.loop_mode = animation::loop_type::REPEAT;

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(int(i) + 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = assets::texture_id(int(last_frame) - 1); i >= first_frame; i = assets::texture_id(int(i) - 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(int(i) + 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);
			frame.sprite.flip_vertically = true;

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = assets::texture_id(int(last_frame) - 1); i >= first_frame; i = assets::texture_id(int(i) - 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);
			frame.sprite.flip_vertically = true;

			anim.frames.push_back(frame);
		}

		return anim;
	}

	animation_response& manager::create(assets::animation_response_id id) {
		animation_response& resp = animation_responses[id];
		return resp;
	}


	particle_effect_response& manager::create(assets::particle_effect_response_id id) {
		auto& resp = particle_effect_responses[id];
		return resp;
	}

	particle_effect& manager::create(assets::particle_effect_id id) {
		auto& resp = particle_effects[id];
		return resp;
	}

	texture_with_image& manager::create(assets::texture_id id, std::string filename) {
		texture_with_image& tex = textures[id];
		tex.set_from_image_file(filename);

		filename.resize(filename.size() - 4);
		filename += "_neon_map.png";

		if (augs::file_exists(filename)) {
			texture_with_image& neon_tex = neon_maps[id];
			neon_tex.set_from_image_file(filename);
		}

		return tex;
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

	void manager::destroy_everything() {
		atlases.clear();
		textures.clear();
		programs.clear();
		shaders.clear();
		animations.clear();
		animation_responses.clear();
		fonts.clear();
	}
}

resources::manager resource_manager;

//namespace assets {
//	texture* texture_id::operator->() const {
//		return &resource_manager.find_texture(id)->tex;
//	}
//}