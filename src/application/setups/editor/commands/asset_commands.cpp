#include "application/setups/editor/commands/asset_commands.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"


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

template struct create_asset_id_command<assets::image_id>;
