#pragma once
#include <tuple>

#include "augs/templates/settable_as_current_mixin.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/texture_atlas/texture_atlas_entry.h"

#include "augs/misc/enum_associative_array.h"
#include "augs/misc/enum_bitset.h"

#include "augs/graphics/shader.h"
#include "augs/graphics/texture.h"

#include "augs/audio/sound_buffer.h"

#include "augs/image/font.h"

#include "game/assets/assets_manager_structs.h"

#include "game/assets/spell_id.h"
#include "game/assets/game_image.h"
#include "game/assets/game_image_id.h"
#include "game/assets/physical_texture_id.h"
#include "game/assets/shader_id.h"
#include "game/assets/program_id.h"
#include "game/assets/font_id.h"
#include "game/assets/animation_id.h"
#include "game/assets/behaviour_tree_id.h"
#include "game/assets/tile_layer_id.h"
#include "game/assets/sound_buffer_id.h"
#include "game/assets/sound_response_id.h"
#include "game/assets/particle_effect_id.h"
#include "game/assets/physical_material_id.h"

#include "game/flyweights/particle_effect.h"
#include "game/flyweights/animation.h"
#include "game/flyweights/tile_layer.h"
#include "game/flyweights/spell_data.h"
#include "game/flyweights/physical_material.h"

#include "game/detail/shape_variant.h"
#include "game/detail/particle_types.h"

class assets_manager : public augs::settable_as_current_mixin<assets_manager> {
	friend struct trait_tests;

	template <class T, class = void>
	struct try_to_get_logical_meta : std::false_type {};

	template <class T>
	struct try_to_get_logical_meta<T, decltype(std::declval<typename T::mapped_type>().get_logical_meta(), void())> : std::true_type {
		typedef decltype(std::declval<typename T::mapped_type>().get_logical_meta()) logical_meta_type;
		
		typedef
			augs::enum_associative_array<
				typename T::key_type,
				logical_meta_type
			>
		type;
	};

	template <class T>
	using does_asset_define_get_logical_meta = try_to_get_logical_meta<T>;

	template <class T>
	struct make_array_of_logical_metas {
		typedef typename try_to_get_logical_meta<T>::type type;
	};

public:
	typedef std::tuple<
		augs::enum_associative_array<assets::animation_id, animation>,
		augs::enum_associative_array<assets::game_image_id, game_image_baked>,
		augs::enum_associative_array<assets::font_id, game_font_baked>,
		augs::enum_associative_array<assets::particle_effect_id, particle_effect>,
		augs::enum_associative_array<assets::spell_id, spell_data>,
		augs::enum_associative_array<assets::tile_layer_id, tile_layer>,
		augs::enum_associative_array<assets::physical_material_id, physical_material>,

		augs::enum_associative_array<assets::shader_id, augs::graphics::shader>,
		augs::enum_associative_array<assets::program_id, augs::graphics::shader_program>,
		augs::enum_associative_array<assets::sound_buffer_id, augs::sound_buffer>,
		augs::enum_associative_array<assets::physical_texture_id, augs::graphics::texture>
	> tuple_of_all_assets;

	typedef replace_list_type_t<
		transform_types_in_list_t<
			filter_types_in_list_t<
				does_asset_define_get_logical_meta,
				tuple_of_all_assets
			>,
			make_array_of_logical_metas
		>,
		augs::trivially_copyable_tuple
	> tuple_of_all_logical_metas_of_assets;

private:
	tuple_of_all_assets all_assets;

	template <class T>
	using find_assets_container_t = find_type_with_key_type_in_list_t<T, tuple_of_all_assets>;
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

	augs::graphics::shader_program& create(
		const assets::program_id program, 
		const assets::shader_id attach_vertex, 
		const assets::shader_id attach_fragment
	);

	template <class id_type>
	decltype(auto) operator[](const id_type id) {
		return std::get<find_assets_container_t<id_type>>(all_assets)[id];
	}

	template <class id_type>
	decltype(auto) operator[](const id_type id) const {
		return std::get<find_assets_container_t<id_type>>(all_assets)[id];
	}

	void write_logical_metas_of_assets_into(cosmos&) const;

	void destroy_everything();
};

inline assets_manager& get_assets_manager() {
	return assets_manager::get_current();
}