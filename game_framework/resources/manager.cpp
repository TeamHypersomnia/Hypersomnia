#include "manager.h"
#include <string>
#include "utilities/file.h"
#include <sstream>

using namespace augs;

namespace assets {
	vec2i get_size(texture_id id) {
		return resource_manager.find(id)->tex.get_size();
	}
}

namespace resources {
	void manager::texture_with_image::set(image img) {
		this->img = img;
		tex.set(&this->img);
	}

	void manager::texture_with_image::set(std::wstring filename) {
		img = image();
		img.from_file(filename);
		tex.set(&img);
	}

	manager::texture_with_image* manager::find(assets::texture_id id) {
		auto it = textures.find(id);
		if (it == textures.end()) return nullptr;

		return &(*it).second;
	}	

	atlas* manager::find(assets::atlas_id id) {
		auto it = atlases.find(id);
		if (it == atlases.end()) return nullptr;

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

	//template<> texture* manager::find(int id) {
	//	return &find_texture(assets::texture_id(id))->tex;
	//}
	//
	//template<> atlas* manager::find(int id) {
	//	return find_atlas(assets::atlas_id(id));
	//}

	atlas& manager::create(assets::atlas_id id, atlas_creation_mode mode) {
		atlases.insert(std::make_pair(id, atlas()));

		atlas& atl = atlases[id];

		if (mode == atlas_creation_mode::FROM_ALL_TEXTURES) {
			for (auto& tex : textures) {
				atl.textures.push_back(&tex.second.tex);
			}
		}

		atl.default_build();

		return atl;
	}

	manager::texture_with_image& manager::create(assets::texture_id id, image img) {
		textures.insert(std::make_pair(id, manager::texture_with_image()));

		manager::texture_with_image& tex = textures[id];
		tex.set(img);

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

	manager::texture_with_image& manager::create(assets::texture_id id, std::wstring filename) {
		textures.insert(std::make_pair(id, manager::texture_with_image()));

		manager::texture_with_image& tex = textures[id];
		tex.set(filename);

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

	void manager::destroy_everything() {
		atlases.clear();
		textures.clear();
		programs.clear();
		shaders.clear();
	}
}

resources::manager resource_manager;

//namespace assets {
//	texture* texture_id::operator->() const {
//		return &resource_manager.find_texture(id)->tex;
//	}
//}