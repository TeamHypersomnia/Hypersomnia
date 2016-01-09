#pragma once
#include "../assets/texture.h"
#include "../assets/atlas.h"
#include "../assets/shader.h"
#include "../assets/program.h"
#include "../assets/animation.h"
#include "../assets/animation_set.h"

#include "../resources/animation.h"

#include "texture_baker/texture_baker.h"
#include "graphics/shader.h"

#include <unordered_map>

namespace resources {
	class manager {
	public:
		class texture {
			/* warning! might get destroyed once some atlas finishes processing bitmaps */
			augs::image img;
			friend class augs::atlas;
		public:
			augs::texture tex;
			
			void set(augs::image img = augs::image());
			void set(std::wstring filename);
		};

		texture* find(assets::texture_id);
		augs::atlas* find(assets::atlas_id);
		augs::graphics::shader_program* find(assets::program_id);

		enum class atlas_creation_mode {
			EMPTY,
			FROM_ALL_TEXTURES
		};

		augs::atlas& create(assets::atlas_id, atlas_creation_mode);
		texture& create(assets::texture_id, std::wstring filename);
		texture& create(assets::texture_id, augs::image img);
		animation& create(assets::animation_id);

		augs::graphics::shader& create(assets::shader_id, std::wstring filename, augs::graphics::shader::type);
		augs::graphics::shader_program& create(assets::program_id, assets::shader_id vertex, assets::shader_id fragment);

		void destroy_everything();

	private:
		std::unordered_map<assets::texture_id, texture> textures;
		std::unordered_map<assets::atlas_id, augs::atlas> atlases;
		std::unordered_map<assets::shader_id, augs::graphics::shader> shaders;
		std::unordered_map<assets::program_id, augs::graphics::shader_program> programs;
	};
}

extern resources::manager resource_manager;