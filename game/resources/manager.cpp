#include "manager.h"
#include <string>
#include "augs/file.h"
#include <sstream>
#include "texture_baker/font.h"

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
	texture_with_image* manager::find(assets::texture_id id) {
		auto it = textures.find(id);
		if (it == textures.end()) return nullptr;

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

	atlas& manager::create(assets::atlas_id id, unsigned atlas_creation_mode_flags) {
		atlases.insert(std::make_pair(id, atlas()));

		atlas& atl = atlases[id];

		if (atlas_creation_mode_flags & atlas_creation_mode::FROM_ALL_TEXTURES) {
			for (auto& tex : textures) {
				atl.textures.push_back(&tex.second.tex);
			}
		}

		if (atlas_creation_mode_flags & atlas_creation_mode::FROM_ALL_FONTS) {
			for (auto& fnt : fonts) {
				fnt.second.add_to_atlas(atl);
			}
		}

		atl.default_build();

		return atl;
	}

	augs::font& manager::create(assets::font_id id) {
		fonts.insert(std::make_pair(id, augs::font()));

		augs::font& font = fonts[id];

		return font;
	}

	texture_with_image& manager::create(assets::texture_id id, image img) {
		textures.insert(std::make_pair(id, texture_with_image()));

		texture_with_image& tex = textures[id];
		tex.set_from_image(img);

		return tex;
	}

	void manager::create_sprites_indexed(assets::texture_id first, assets::texture_id last, std::wstring filename_preffix) {
		for (assets::texture_id i = first; i < last; i = assets::texture_id(i + 1)) {
			std::wostringstream filename;
			filename << filename_preffix << L"_" << int(i - first + 1) << L".png";

			create(i, filename.str());
		}
	}

	animation& manager::create(assets::animation_id id) {
		animations.insert(std::make_pair(id, animation()));

		animation& anim = animations[id];
		return anim;
	}

	animation& manager::create(assets::animation_id id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms, 
		resources::animation::loop_type loop_mode) {
		
		animation& anim = create(id);
		anim.loop_mode = loop_mode;

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(i+1)) {
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

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(i + 1)) {
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

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(i + 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = assets::texture_id(last_frame - 1); i >= first_frame; i = assets::texture_id(i - 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(i + 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);
			frame.sprite.flip_vertically = true;

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = assets::texture_id(last_frame - 1); i >= first_frame; i = assets::texture_id(i - 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);
			frame.sprite.flip_vertically = true;

			anim.frames.push_back(frame);
		}

		return anim;
	}

	animation_response& manager::create(assets::animation_response_id id) {
		animation_responses.insert(std::make_pair(id, animation_response()));

		animation_response& resp = animation_responses[id];
		return resp;
	}


	particle_effect_response& manager::create(assets::particle_effect_response_id id) {
		particle_effect_responses.insert(std::make_pair(id, particle_effect_response()));

		auto& resp = particle_effect_responses[id];
		return resp;
	}

	particle_effect& manager::create(assets::particle_effect_id id) {
		particle_effects.insert(std::make_pair(id, particle_effect()));

		auto& resp = particle_effects[id];
		return resp;
	}

	texture_with_image& manager::create(assets::texture_id id, std::wstring filename) {
		textures.insert(std::make_pair(id, texture_with_image()));

		texture_with_image& tex = textures[id];
		tex.set_from_image_file(filename);

		return tex;
	}

	graphics::shader& manager::create(assets::shader_id id, std::wstring filename, augs::graphics::shader::type type) {
		shaders.insert(std::make_pair(id, graphics::shader()));

		graphics::shader& sh = shaders[id];
		sh.create(type, get_file_contents(filename));

		return sh;
	}

	graphics::shader_program& manager::create(assets::program_id program_id, assets::shader_id vertex_id, assets::shader_id fragment_id) {
		programs.insert(std::make_pair(program_id, graphics::shader_program()));

		graphics::shader_program& p = programs[program_id];
		p.create(); 
		p.attach(shaders[vertex_id]);
		p.attach(shaders[fragment_id]);
		p.build();

		return p;
	}
	
	behaviour_tree& manager::create(assets::behaviour_tree_id id) {
		behaviour_trees.insert(std::make_pair(id, behaviour_tree()));

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