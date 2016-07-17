#pragma once
#include <unordered_map>

#include "game/assets/texture_id.h"
#include "game/assets/atlas_id.h"
#include "game/assets/shader_id.h"
#include "game/assets/program_id.h"
#include "game/assets/font_id.h"
#include "game/assets/animation_id.h"
#include "game/assets/animation_response_id.h"
#include "game/assets/particle_effect_id.h"
#include "game/assets/particle_effect_response_id.h"
#include "game/assets/behaviour_tree_id.h"

#include "game/resources/animation.h"
#include "game/resources/animation_response.h"
#include "game/resources/texture_with_image.h"
#include "game/resources/particle_effect.h"
#include "game/resources/particle_effect_response.h"
#include "game/resources/behaviour_tree.h"

#include "texture_baker/texture_baker.h"
#include "texture_baker/font.h"
#include "graphics/shader.h"

namespace resources {
	class manager {
	public:
		enum atlas_creation_mode {
			EMPTY = 0,
			FROM_ALL_TEXTURES = 1,
			FROM_ALL_FONTS = 2
		};

		augs::atlas& create(assets::atlas_id, unsigned atlas_creation_mode_flags);
		augs::font& create(assets::font_id);
		texture_with_image& create(assets::texture_id, std::string filename);
		texture_with_image& create(assets::texture_id, augs::image img);

		void create_sprites_indexed(assets::texture_id first, assets::texture_id last, std::string filename_preffix);

		animation& create(assets::animation_id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms,
			resources::animation::loop_type = resources::animation::INVERSE
			);

		animation& create_inverse(assets::animation_id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms);
		animation& create_inverse_with_flip(assets::animation_id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms);

		animation& create(assets::animation_id);
		resources::animation_response& create(assets::animation_response_id);

		particle_effect& create(assets::particle_effect_id);
		particle_effect_response& create(assets::particle_effect_response_id);

		augs::graphics::shader& create(assets::shader_id, std::string filename, augs::graphics::shader::type);
		augs::graphics::shader_program& create(assets::program_id, assets::shader_id vertex, assets::shader_id fragment);

		behaviour_tree& create(assets::behaviour_tree_id);

		texture_with_image* find(assets::texture_id);
		augs::font* find(assets::font_id);
		augs::atlas* find(assets::atlas_id);
		augs::graphics::shader_program* find(assets::program_id);
		animation* find(assets::animation_id);
		animation_response* find(assets::animation_response_id);
		particle_effect* find(assets::particle_effect_id);
		particle_effect_response* find(assets::particle_effect_response_id);
		behaviour_tree* find(assets::behaviour_tree_id);

		void destroy_everything();

	private:

		std::unordered_map<assets::particle_effect_id, particle_effect> particle_effects;
		std::unordered_map<assets::particle_effect_response_id, particle_effect_response> particle_effect_responses;
		std::unordered_map<assets::animation_response_id, animation_response> animation_responses;
		std::unordered_map<assets::animation_id, animation> animations;
		std::unordered_map<assets::texture_id, texture_with_image> textures;
		std::unordered_map<assets::font_id, augs::font> fonts;
		std::unordered_map<assets::atlas_id, augs::atlas> atlases;
		std::unordered_map<assets::shader_id, augs::graphics::shader> shaders;
		std::unordered_map<assets::program_id, augs::graphics::shader_program> programs;
		std::unordered_map<assets::behaviour_tree_id, behaviour_tree> behaviour_trees;
	};
}

extern resources::manager resource_manager;