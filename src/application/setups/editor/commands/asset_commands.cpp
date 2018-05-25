#include "view/get_asset_pool.h"
#include "application/setups/editor/commands/asset_commands.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

#include "augs/readwrite/byte_readwrite.h"

template <class I>
std::string create_unpathed_asset_id_command<I>::describe() const {
	return typesafe_sprintf("Created a new %x", uncapitalize_first(format_field_name(get_type_name_strip_namespace<I>())));
}

template <class I>
void create_unpathed_asset_id_command<I>::redo(const editor_command_input in) {
	base::redo(access_asset_pool<I>(in, {}));
}

template <class I>
void create_unpathed_asset_id_command<I>::undo(const editor_command_input in) {
	base::undo(access_asset_pool<I>(in, {}));
}

template <class I>
std::string create_pathed_asset_id_command<I>::describe() const {
	return typesafe_sprintf("Started tracking asset file: %x", construct_from.get_source_path().to_display());
}

template <class I>
void create_pathed_asset_id_command<I>::redo(const editor_command_input in) {
	base::redo(get_asset_pool<I>(in), construct_from);
}

template <class I>
void create_pathed_asset_id_command<I>::undo(const editor_command_input in) {
	base::undo(get_asset_pool<I>(in));
}

template <class I>
std::string forget_asset_id_command<I>::describe() const {
	return built_description;
}

template <class I>
void forget_asset_id_command<I>::redo(const editor_command_input in) {
	base::redo(access_asset_pool<I>(in, {}));
}

template <class I>
void forget_asset_id_command<I>::undo(const editor_command_input in) {
	base::undo(access_asset_pool<I>(in, {}));
}

template <class I>
std::string duplicate_asset_command<I>::describe() const {
	return typesafe_sprintf("Duplicated %x", uncapitalize_first(format_field_name(get_type_name_strip_namespace<I>())));
}

template <class I>
void duplicate_asset_command<I>::redo(const editor_command_input in) {
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

template struct create_unpathed_asset_id_command<assets::torso_animation_id>;
template struct forget_asset_id_command<assets::torso_animation_id>;
template struct duplicate_asset_command<assets::torso_animation_id>;
template struct change_asset_property_command<assets::torso_animation_id>;

template struct create_unpathed_asset_id_command<assets::legs_animation_id>;
template struct forget_asset_id_command<assets::legs_animation_id>;
template struct duplicate_asset_command<assets::legs_animation_id>;
template struct change_asset_property_command<assets::legs_animation_id>;
