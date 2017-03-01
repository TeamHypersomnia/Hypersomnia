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
#include "game/assets/tile_layer_id.h"
#include "game/assets/sound_buffer_id.h"
#include "game/assets/sound_response_id.h"

#include "game/resources/animation.h"
#include "game/resources/animation_response.h"
#include "game/resources/sound_response.h"
#include "augs/texture_atlas/texture_with_image.h"
#include "game/resources/particle_effect.h"
#include "game/resources/particle_effect_response.h"
#include "game/resources/behaviour_tree.h"
#include "game/resources/tile_layer.h"

#include "augs/texture_atlas/texture_atlas.h"
#include "augs/image/font.h"
#include "augs/graphics/shader.h"
#include "augs/audio/sound_buffer.h"

#include "augs/misc/enum_associative_array.h"

namespace resources {
	class manager {
	public:
		enum atlas_creation_mode {
			EMPTY = 0,
			FROM_ALL_TEXTURES = 1,
			FROM_ALL_FONTS = 2
		};

		sound_response& create(const assets::sound_response_id);
		augs::sound_buffer& create(const assets::sound_buffer_id);
		augs::texture_atlas& create(assets::atlas_id, unsigned atlas_creation_mode_flags);
		augs::font& create(assets::font_id);
		
		void associate_neon_map(
			const assets::texture_id take_neon_map_from, 
			const assets::texture_id target_to_be_assigned
		);

		augs::texture_with_image& create(
			const assets::texture_id, 
			std::string filename,
			const bool generate_desaturated = false
		);

		augs::texture_with_image& create(const assets::texture_id, augs::image img);

		void create_sprites_indexed(assets::texture_id first, assets::texture_id last, std::string filename_preffix);

		animation& create(assets::animation_id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms,
			resources::animation::loop_type = resources::animation::INVERSE
			);

		animation& create_inverse(assets::animation_id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms);
		animation& create_inverse_with_flip(assets::animation_id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms);

		animation& create(assets::animation_id at);
		resources::animation_response& create(assets::animation_response_id at);

		particle_effect& create(assets::particle_effect_id at);
		particle_effect_response& create(assets::particle_effect_response_id at);

		augs::graphics::shader& create(assets::shader_id, std::string filename, augs::graphics::shader::type);
		augs::graphics::shader_program& create(assets::program_id, assets::shader_id vertex, assets::shader_id fragment);

		behaviour_tree& create(assets::behaviour_tree_id);
		tile_layer& create(assets::tile_layer_id);

		augs::texture_with_image* find(const assets::texture_id);
		augs::texture_with_image* find_neon_map(const assets::texture_id);
		augs::texture_with_image* find_desaturated(const assets::texture_id);
		augs::font* find(assets::font_id);
		augs::texture_atlas* find(assets::atlas_id);
		augs::graphics::shader_program* find(assets::program_id);
		animation* find(assets::animation_id);
		animation_response* find(assets::animation_response_id);
		particle_effect* find(assets::particle_effect_id);
		particle_effect_response* find(assets::particle_effect_response_id);
		behaviour_tree* find(assets::behaviour_tree_id);
		tile_layer* find(assets::tile_layer_id);
		augs::sound_buffer* manager::find(const assets::sound_buffer_id id);
		sound_response* manager::find(const assets::sound_response_id id);

		void destroy_everything();

	private:

		augs::enum_associative_array<assets::particle_effect_id, particle_effect> particle_effects;
		augs::enum_associative_array<assets::particle_effect_response_id, particle_effect_response> particle_effect_responses;
		augs::enum_associative_array<assets::animation_response_id, animation_response> animation_responses;
		augs::enum_associative_array<assets::animation_id, animation> animations;
		augs::enum_associative_array<assets::texture_id, augs::texture_with_image> textures;
		augs::enum_associative_array<assets::texture_id, augs::texture_with_image> neon_maps;
		augs::enum_associative_array<assets::texture_id, augs::texture_with_image> desaturated_textures;
		augs::enum_associative_array<assets::font_id, augs::font> fonts;
		augs::enum_associative_array<assets::atlas_id, augs::texture_atlas> atlases;
		augs::enum_associative_array<assets::shader_id, augs::graphics::shader> shaders;
		augs::enum_associative_array<assets::program_id, augs::graphics::shader_program> programs;
		augs::enum_associative_array<assets::behaviour_tree_id, behaviour_tree> behaviour_trees;
		augs::enum_associative_array<assets::tile_layer_id, tile_layer> tile_layers;
		augs::enum_associative_array<assets::sound_buffer_id, augs::sound_buffer> sound_buffers;
		augs::enum_associative_array<assets::sound_response_id, sound_response> sound_responses;
	};
}

resources::manager& get_resource_manager();