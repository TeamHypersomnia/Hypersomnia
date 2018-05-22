#include "application/setups/editor/commands/asset_commands.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/detail/get_asset_pool.h"
#include "application/intercosm.h"

#include "augs/readwrite/byte_readwrite.h"

template <class I>
std::string create_asset_id_command<I>::describe() const {
	return typesafe_sprintf("Started tracking asset file: %x", use_path.to_display());
}

template <class I>
void create_asset_id_command<I>::redo(const editor_command_input in) {
	auto& new_object = base::redo(get_asset_pool<I>(in));
	new_object.set_source_path(use_path);
}

template <class I>
void create_asset_id_command<I>::undo(const editor_command_input in) {
	base::undo(get_asset_pool<I>(in));
}

template <class I>
std::string forget_asset_id_command<I>::describe() const {
	return built_description;
}

template <class I>
void forget_asset_id_command<I>::redo(const editor_command_input in) {
	base::redo(get_asset_pool<I>(in));
}

template <class I>
void forget_asset_id_command<I>::undo(const editor_command_input in) {
	base::undo(get_asset_pool<I>(in));
}

template struct create_asset_id_command<assets::image_id>;
template struct forget_asset_id_command<assets::image_id>;
template struct change_asset_property_command<assets::image_id>;

template struct create_asset_id_command<assets::sound_id>;
template struct forget_asset_id_command<assets::sound_id>;
template struct change_asset_property_command<assets::sound_id>;
