#include "game/assets/animation_templates.h"

#include "view/get_asset_pool.h"
#include "application/setups/debugger/commands/asset_commands.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/intercosm.h"
#include "application/setups/debugger/detail/find_free_name.h"

#include "augs/misc/pool/pool_allocate.h"
#include "augs/readwrite/byte_readwrite.h"

template <class T, class = void>
struct has_get_name : std::false_type {};

template <class T>
struct has_get_name<T, decltype(std::declval<T&>().get_name(), void())> : std::true_type {};

template <class T>
constexpr bool has_get_name_v = has_get_name<T>::value; 


template <class T, class = void>
struct has_emissions : std::false_type {};

template <class T>
struct has_emissions<T, decltype(std::declval<T&>().emissions, void())> : std::true_type {};

template <class T>
constexpr bool has_emissions_v = has_emissions<T>::value; 


template <class I>
std::string create_unpathed_asset_id_command<I>::describe() const {
	return typesafe_sprintf("Created a new %x", uncapitalize_first(format_field_name(get_type_name_strip_namespace<I>())));
}

template <class I>
void create_unpathed_asset_id_command<I>::redo(const debugger_command_input in) {
	auto& pool = access_asset_pool<I>(in, {});
	auto& new_object = base::redo(pool);

	using O = remove_cref<decltype(new_object)>;

	if constexpr(has_get_name_v<O>) {
		new_object.name = ::find_free_name(pool, get_type_name_strip_namespace<O>() + "-");
	}

	if constexpr(has_frames_v<O>) {
		/* Always create a single frame to avoid crashes */

		frame_type_t<O> f;
		f.image_id = in.get_viewable_defs().image_definitions.get_nth_id(0);
		new_object.frames.push_back(f);
	}

	if constexpr(has_emissions_v<O>) {
		/* Always create a single emission to avoid crashes */
		new_object.emissions.emplace_back();
	}
}

template <class I>
void create_unpathed_asset_id_command<I>::undo(const debugger_command_input in) {
	base::undo(access_asset_pool<I>(in, {}));
}

template <class I>
std::string create_pathed_asset_id_command<I>::describe() const {
	return typesafe_sprintf("Started tracking asset file: %x", construct_from.get_source_path().filename_first());
}

template <class I>
void create_pathed_asset_id_command<I>::redo(const debugger_command_input in) {
	base::redo(get_asset_pool<I>(in), construct_from);

	if constexpr(std::is_same_v<I, assets::image_id>) {
		in.folder.commanded->work.update_offsets_of(base::get_allocated_id());
	}
}

template <class I>
void create_pathed_asset_id_command<I>::undo(const debugger_command_input in) {
	base::undo(get_asset_pool<I>(in));
}

template <class I>
std::string forget_asset_id_command<I>::describe() const {
	return built_description;
}

template <class I>
void forget_asset_id_command<I>::redo(const debugger_command_input in) {
	base::redo(access_asset_pool<I>(in, {}));
}

template <class I>
void forget_asset_id_command<I>::undo(const debugger_command_input in) {
	base::undo(access_asset_pool<I>(in, {}));
}

template <class I>
std::string duplicate_asset_command<I>::describe() const {
	return typesafe_sprintf("Cloned %x", uncapitalize_first(format_field_name(get_type_name_strip_namespace<I>())));
}

template <class I>
void duplicate_asset_command<I>::redo(const debugger_command_input in) {
	base::redo_and_copy(access_asset_pool<I>(in, {}), duplicate_from);
}

template struct create_pathed_asset_id_command<assets::image_id>;
template struct forget_asset_id_command<assets::image_id>;
template struct change_asset_property_command<assets::image_id>;

template struct create_pathed_asset_id_command<assets::sound_id>;
template struct forget_asset_id_command<assets::sound_id>;
template struct change_asset_property_command<assets::sound_id>;

template struct create_unpathed_asset_id_command<assets::plain_animation_id>;
template struct forget_asset_id_command<assets::plain_animation_id>;
template struct duplicate_asset_command<assets::plain_animation_id>;
template struct change_asset_property_command<assets::plain_animation_id>;

template struct create_unpathed_asset_id_command<assets::particle_effect_id>;
template struct forget_asset_id_command<assets::particle_effect_id>;
template struct duplicate_asset_command<assets::particle_effect_id>;
template struct change_asset_property_command<assets::particle_effect_id>;
