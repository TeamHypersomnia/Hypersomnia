#pragma once
// This is a performance switch.
// Asset manager being global means one indirection less (and thus one cache fetch less) when accessing resources.
// This may prove very beneficial, for example when rendering many sprites.
#include <tuple>
#include <memory>
#include "game/build_settings.h"

#include "augs/templates/settable_as_current_mixin.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/texture_atlas/texture_atlas_entry.h"

#include "augs/misc/enum_associative_array.h"

#include "augs/graphics/shader.h"
#include "augs/graphics/texture.h"

#include "augs/audio/sound_buffer.h"

#include "augs/image/font.h"

#include "game/assets/game_image_structs.h"

#include "game/assets/game_image.h"
#include "game/assets/game_image_id.h"
#include "game/assets/gl_texture_id.h"
#include "game/assets/shader_program_id.h"
#include "game/assets/font_id.h"
#include "game/assets/animation_id.h"
#include "game/assets/behaviour_tree_id.h"
#include "game/assets/tile_layer_id.h"
#include "game/assets/sound_buffer_id.h"
#include "game/assets/particle_effect_id.h"
#include "game/assets/physical_material_id.h"
#include "game/assets/recoil_player_id.h"

#include "game/assets/particle_effect.h"
#include "game/assets/animation.h"
#include "game/assets/tile_layer.h"
#include "game/assets/physical_material.h"
#include "game/assets/recoil_player.h"

#include "game/detail/particle_types.h"

class assets_manager;

template <class T, class = void>
struct try_to_get_logical_meta : std::false_type {};

template <class T>
struct try_to_get_logical_meta<
	T, 
	decltype(std::declval<typename T::mapped_type>().get_logical_meta(std::declval<assets_manager>()), void())
> : std::true_type {
	using logical_meta_type = decltype(std::declval<typename T::mapped_type>().get_logical_meta(std::declval<assets_manager>()));
	
	using type = augs::enum_associative_array<
		typename T::key_type,
		logical_meta_type
	>;
};

template <class T>
using does_asset_define_get_logical_meta = try_to_get_logical_meta<T>;

template <class T>
struct make_container_of_logical_metas {
	using type = typename try_to_get_logical_meta<T>::type;
};

using tuple_of_all_assets = std::tuple<
	augs::enum_associative_array<assets::animation_id, animation>,
	augs::enum_associative_array<assets::game_image_id, game_image_baked>,
	augs::enum_associative_array<assets::font_id, game_font_baked>,
	augs::enum_associative_array<assets::particle_effect_id, particle_effect>,
	augs::enum_associative_array<assets::tile_layer_id, tile_layer>,
	augs::enum_associative_array<assets::physical_material_id, physical_material>,
	augs::enum_associative_array<assets::recoil_player_id, recoil_player>,

	augs::enum_associative_array<assets::shader_program_id, augs::graphics::shader_program>,
	augs::enum_associative_array<assets::sound_buffer_id, augs::sound_buffer>,
	augs::enum_associative_array<assets::gl_texture_id, augs::graphics::texture>
>;

using tuple_of_all_logical_metas_of_assets = replace_list_type_t<
	transform_types_in_list_t<
		filter_types_in_list_t<
			does_asset_define_get_logical_meta,
			tuple_of_all_assets
		>,
		make_container_of_logical_metas
	>,
	augs::trivially_copyable_tuple
>;

template <class derived>
struct asset_getters_mixin {
	using base = asset_getters_mixin<derived>;

	template <class id_type>
	decltype(auto) find(const id_type id) {
		auto& self = *static_cast<derived*>(this);
		return found_or_nullptr(self.get_store_by<id_type>(), id);
	}

	template <class id_type>
	decltype(auto) find(const id_type id) const {
		const auto& self = *static_cast<const derived*>(this);
		return found_or_nullptr(self.get_store_by<id_type>(), id);
	}

	template <class id_type>
	decltype(auto) operator[](const id_type id) {
		auto& self = *static_cast<derived*>(this);

		return self.get_store_by<id_type>()[id];
	}

	template <class id_type>
	decltype(auto) at(const id_type id) {
		auto& self = *static_cast<derived*>(this);

		return self.get_store_by<id_type>().at(id);
	}

	template <class id_type>
	decltype(auto) at(const id_type id) const {
		const auto& self = *static_cast<const derived*>(this);

		return self.get_store_by<id_type>().at(id);
	}
};

struct all_logical_metas_of_assets : asset_getters_mixin<all_logical_metas_of_assets> {
	tuple_of_all_logical_metas_of_assets all;

	template <class T>
	auto& get_store_by() {
		return get_container_with_key_type<T>(all);
	}

	template <class T>
	auto& get_store_by() const {
		return get_container_with_key_type<T>(all);
	}
};

class assets_manager : 
#if !ONLY_ONE_GLOBAL_ASSETS_MANAGER
	public augs::settable_as_current_mixin<assets_manager>,
#endif
	public asset_getters_mixin<assets_manager>
{
	friend struct base;
	tuple_of_all_assets all;

	bool set_as_current_impl() { return true; }
	void set_current_to_none_impl() {}
public:
#if !ONLY_ONE_GLOBAL_ASSETS_MANAGER
	template <class P>
	assets_manager(P&& populate) {
		populate(*this);
	}
#endif

	void load_baked_metadata(
		const game_image_definitions&,
		const game_font_definitions&,
		const atlases_regeneration_output&
	);

	void load_requisite(const assets::gl_texture_id);

	// unique_ptr to avoid accidental stack overflow
	std::unique_ptr<all_logical_metas_of_assets> generate_logical_metas_of_assets() const;

	template <class T>
	auto& get_store_by() {
		return get_container_with_key_type<T>(all);
	}

	template <class T>
	auto& get_store_by() const {
		return get_container_with_key_type<T>(all);
	}
};

#if ONLY_ONE_GLOBAL_ASSETS_MANAGER
extern assets_manager global_instance;
#endif

inline assets_manager& get_assets_manager() {
#if ONLY_ONE_GLOBAL_ASSETS_MANAGER
	return global_instance;
#else
	return assets_manager::get_current();
#endif
}