#include "manager.h"
#include <string>
#include "utilities/file.h"

using namespace augs;

namespace resources {
	void manager::texture::set(image img) {
		this->img = img;
		tex.set(&this->img);
	}

	void manager::texture::set(std::wstring filename) {
		img = image();
		img.from_file(filename);
		tex.set(&img);
	}

	manager::texture* manager::find(assets::texture_id id) {
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

	manager::texture& manager::create(assets::texture_id id, image img) {
		textures.insert(std::make_pair(id, manager::texture()));

		manager::texture& tex = textures[id];
		tex.set(img);

		return tex;
	}

	manager::texture& manager::create(assets::texture_id id, std::wstring filename) {
		textures.insert(std::make_pair(id, manager::texture()));

		manager::texture& tex = textures[id];
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