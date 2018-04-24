#include "application/setups/editor/commands/asset_commands.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"
#include "view/viewables/image_meta.h"
#include "view/viewables/get_viewable_pool.h"

#include "augs/readwrite/byte_readwrite.h"

template <class I>
std::string create_asset_id_command<I>::describe() const {
	return typesafe_sprintf("Started tracking image: %x", use_path.to_display());
}

template <class I>
void create_asset_id_command<I>::redo(const editor_command_input in) {
	auto& work = *in.folder.work;

	const auto previous_id = allocated_id;

	auto validate = [previous_id](const auto new_id) {
		if (previous_id.is_set()) {
			ensure_eq(new_id, previous_id);
		}
	};

	{
		auto& definitions = get_viewable_pool<I>(work.viewables);

		const auto allocation = definitions.allocate();
		const auto new_id = allocation.key;
		allocation.object.set_source_path(use_path);

		validate(new_id);

		allocated_id = assets::image_id(new_id);
	}
}

template <class I>
void create_asset_id_command<I>::undo(const editor_command_input in) {
	auto& work = *in.folder.work;

	auto& definitions = get_viewable_pool<I>(work.viewables);
	definitions.undo_last_allocate(allocated_id);
}

template <class I>
std::string forget_asset_id_command<I>::describe() const {
	return built_description;
}

template <class I>
void forget_asset_id_command<I>::redo(const editor_command_input in) {
	auto& work = *in.folder.work;

	ensure(forgotten_content.empty());

	auto s = augs::ref_memory_stream(forgotten_content);

	auto& definitions = get_viewable_pool<I>(work.viewables);

	augs::write_bytes(s, definitions[forgotten_id]);
	undo_free_input = *definitions.free(forgotten_id);
}

template <class I>
void forget_asset_id_command<I>::undo(const editor_command_input in) {
	auto& work = *in.folder.work;

	auto s = augs::cref_memory_stream(forgotten_content);

	auto& definitions = get_viewable_pool<I>(work.viewables);

	typename std::decay_t<decltype(definitions)>::mapped_type def;
	augs::read_bytes(s, def);
	definitions.undo_free(undo_free_input, std::move(def));

	forgotten_content.clear();
}

template struct create_asset_id_command<assets::image_id>;
template struct forget_asset_id_command<assets::image_id>;
template struct change_asset_property_command<assets::image_id>;
