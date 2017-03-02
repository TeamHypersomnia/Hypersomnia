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
#include "augs/graphics/texture.h"
#include "augs/audio/sound_buffer.h"

#include "augs/misc/enum_associative_array.h"
#include "augs/misc/enum_bitset.h"

#include "game/resources/atlas_content_structs.h"


namespace resources {
	class manager {
	public:
		void regenerate_atlases_and_load_baked_metadata(const requested_atlas_resources&);

		void set(
			const assets::texture_id,
			const image_usage_settings settings
		);

		void associate_neon_map(
			const assets::texture_id take_neon_map_from,
			const assets::texture_id target_to_be_assigned
		);

		image_usage_settings get_usage_settings(const assets::texture_id) const;

		void create(
			const assets::atlas_id,
			const std::string& source_filename
		);

		source_image_baked* find(const assets::texture_id);
		augs::font_metadata* find(const assets::font_id);
		augs::graphics::texture* find(const assets::atlas_id);

		sound_response& create(const assets::sound_response_id);
		augs::sound_buffer& create(const assets::sound_buffer_id);

		animation& create(
			const assets::animation_id, 
			const assets::texture_id first_frame, 
			const assets::texture_id last_frame, 
			const float frame_duration_ms,
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
		augs::enum_associative_array<assets::texture_id, source_image_baked> source_images_baked;
		augs::enum_associative_array<assets::font_id, augs::font_metadata> source_fonts_baked;

		augs::enum_associative_array<assets::texture_id, image_usage_settings> usage_settings;

		augs::enum_associative_array<assets::atlas_id, augs::graphics::texture> physical_textures;

		augs::enum_associative_array<assets::particle_effect_id, particle_effect> particle_effects;
		augs::enum_associative_array<assets::particle_effect_response_id, particle_effect_response> particle_effect_responses;
		augs::enum_associative_array<assets::animation_response_id, animation_response> animation_responses;
		augs::enum_associative_array<assets::animation_id, animation> animations;
		
		augs::enum_associative_array<assets::shader_id, augs::graphics::shader> shaders;
		augs::enum_associative_array<assets::program_id, augs::graphics::shader_program> programs;
		augs::enum_associative_array<assets::behaviour_tree_id, behaviour_tree> behaviour_trees;
		augs::enum_associative_array<assets::tile_layer_id, tile_layer> tile_layers;
		augs::enum_associative_array<assets::sound_buffer_id, augs::sound_buffer> sound_buffers;
		augs::enum_associative_array<assets::sound_response_id, sound_response> sound_responses;
	};
}

resources::manager& get_resource_manager();