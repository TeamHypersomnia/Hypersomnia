#pragma once
#include <unordered_map>

#include "game/assets/game_image_id.h"
#include "game/assets/physical_texture_id.h"
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
#include "game/resources/particle_effect.h"
#include "game/resources/particle_effect_response.h"
#include "game/resources/behaviour_tree.h"
#include "game/resources/tile_layer.h"

#include "augs/image/font.h"
#include "augs/graphics/shader.h"
#include "augs/graphics/texture.h"
#include "augs/audio/sound_buffer.h"

#include "augs/misc/enum_associative_array.h"
#include "augs/misc/enum_bitset.h"

#include "application/content_generation/atlas_content_structs.h"
#include "augs/texture_atlas/texture_atlas_entry.h"
#include "augs/templates/settable_as_current_mixin.h"

enum class texture_map_type {
	DIFFUSE,
	NEON,
	DESATURATED,

	COUNT
};

struct game_image_usage_settings {
	struct {
		bool flip_horizontally = false;
		bool flip_vertically = false;
		std::array<padding_byte, 2> pad;

		vec2 bbox_expander;
	} gui;
};

struct game_image_request {
	augs::enum_array<source_image_loading_input, texture_map_type> texture_maps;

	std::string polygonization_filename;
	game_image_usage_settings settings;
};

typedef source_font_loading_input game_font_request;

struct game_image_baked {
	augs::enum_array<augs::texture_atlas_entry, texture_map_type> texture_maps;

	std::vector<vec2u> polygonized;
	game_image_usage_settings settings;

	vec2u get_size() const {
		return texture_maps[texture_map_type::DIFFUSE].get_size();
	}
};

typedef augs::baked_font game_font_baked;

typedef std::unordered_map<assets::game_image_id, game_image_request> game_image_requests;
typedef std::unordered_map<assets::font_id, game_font_request> game_font_requests;

struct atlases_regeneration_output;

namespace resources {
	class manager : public augs::settable_as_current_mixin<manager> {
	public:
		void load_baked_metadata(
			const game_image_requests&,
			const game_font_requests&,
			const atlases_regeneration_output&
		);

		void create(
			const assets::physical_texture_id,
			const bool load_as_binary
		);

		game_image_baked* find(const assets::game_image_id);
		game_font_baked* find(const assets::font_id);
		
		augs::graphics::texture* find(const assets::physical_texture_id);

		augs::sound_buffer& create(const assets::sound_buffer_id);

		animation& create(
			const assets::animation_id, 
			const assets::game_image_id first_frame, 
			const assets::game_image_id last_frame, 
			const float frame_duration_ms,
			resources::animation::loop_type = resources::animation::INVERSE
		);

		animation& create_inverse(assets::animation_id, assets::game_image_id first_frame, assets::game_image_id last_frame, float frame_duration_ms);
		animation& create_inverse_with_flip(assets::animation_id, assets::game_image_id first_frame, assets::game_image_id last_frame, float frame_duration_ms);

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

		void destroy_everything();

	private:
		augs::enum_associative_array<assets::game_image_id, game_image_baked> baked_game_images;
		augs::enum_associative_array<assets::font_id, game_font_baked> baked_game_fonts;

		augs::enum_associative_array<assets::physical_texture_id, augs::graphics::texture> physical_textures;

		augs::enum_associative_array<assets::particle_effect_id, particle_effect> particle_effects;
		augs::enum_associative_array<assets::particle_effect_response_id, particle_effect_response> particle_effect_responses;
		augs::enum_associative_array<assets::animation_response_id, animation_response> animation_responses;
		augs::enum_associative_array<assets::animation_id, animation> animations;
		
		augs::enum_associative_array<assets::shader_id, augs::graphics::shader> shaders;
		augs::enum_associative_array<assets::program_id, augs::graphics::shader_program> programs;
		augs::enum_associative_array<assets::behaviour_tree_id, behaviour_tree> behaviour_trees;
		augs::enum_associative_array<assets::tile_layer_id, tile_layer> tile_layers;
		augs::enum_associative_array<assets::sound_buffer_id, augs::sound_buffer> sound_buffers;
	};
}

inline resources::manager& get_resource_manager() {
	return resources::manager::get_current();
}