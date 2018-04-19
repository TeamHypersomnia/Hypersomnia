#include "application/setups/editor/commands/asset_commands.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

#include "augs/readwrite/byte_readwrite.h"

template <class I>
std::string create_asset_id_command<I>::describe() const {
	return typesafe_sprintf("Started tracking image: %x", augs::to_display_path(use_path));
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

	if constexpr(std::is_same_v<I, assets::image_id>) {
		{
			auto& loadables = work.viewables.image_loadables;

			const auto allocation = loadables.allocate();
			const auto new_id = allocation.key;
			allocation.object.source_image_path = use_path;

			validate(new_id);

			allocated_id = assets::image_id(new_id);
		}

		{
			auto& metas = work.viewables.image_metas;
			const auto new_id = metas.allocate().key;
			validate(new_id);
		}
	}
	else {
		static_assert(always_false_v<I>, "Unsupported id type.");
	}
}

template <class I>
void create_asset_id_command<I>::undo(const editor_command_input in) {
	auto& work = *in.folder.work;

	if constexpr(std::is_same_v<I, assets::image_id>) {
		{
			auto& loadables = work.viewables.image_loadables;
			loadables.undo_last_allocate(allocated_id);
		}

		{
			auto& metas = work.viewables.image_metas;
			metas.undo_last_allocate(allocated_id);
		}
	}
	else {
		static_assert(always_false_v<I>, "Unsupported id type.");
	}
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
	auto save = [&s](const auto& what) {
		augs::write_bytes(s, what);
	};

	if constexpr(std::is_same_v<I, assets::image_id>) {
		{
			auto& loadables = work.viewables.image_loadables;
			save(loadables[forgotten_id]);
			undo_free_input = *loadables.free(forgotten_id);
		}

		{
			auto& metas = work.viewables.image_metas;

			auto s = augs::ref_memory_stream(forgotten_content);
			save(metas[forgotten_id]);
			metas.free(forgotten_id);
		}
	}
	else {
		static_assert(always_false_v<I>, "Unsupported id type.");
	}
}

template <class I>
void forget_asset_id_command<I>::undo(const editor_command_input in) {
	auto& work = *in.folder.work;

	auto s = augs::cref_memory_stream(forgotten_content);

	auto load = [&s](auto& to) {
		augs::read_bytes(s, to);
	};

	if constexpr(std::is_same_v<I, assets::image_id>) {
		{
			auto& loadables = work.viewables.image_loadables;
			image_loadables_def def;
			load(def);
			loadables.undo_free(undo_free_input, std::move(def));
		}

		{
			auto& metas = work.viewables.image_metas;
			image_meta def;
			load(def);
			metas.undo_free(undo_free_input, std::move(def));
		}
	}
	else {
		static_assert(always_false_v<I>, "Unsupported id type.");
	}

	forgotten_content.clear();
}

template struct create_asset_id_command<assets::image_id>;
template struct forget_asset_id_command<assets::image_id>;

template struct change_asset_property_command<assets::image_id, false>;
template struct change_asset_property_command<assets::image_id, true>;
