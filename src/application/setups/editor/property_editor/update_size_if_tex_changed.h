#pragma once
#include "game/assets/ids/asset_ids.h"
#include "view/viewables/image_cache.h"

#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/property_editor/fae/fae_tree_structs.h"
#include "application/setups/editor/commands/change_flavour_property_command.h"
#include "application/setups/editor/detail/field_address.h"

template <class T, class C>
void update_size_if_tex_changed(
	const edit_invariant_input,
	const T&,
	const C& 
) {}

inline void update_size_if_tex_changed(
	const edit_invariant_input in,
	const invariants::sprite&,
	const assets::image_id new_id
) {
	const auto fae_in = in.fae_in;
	const auto cpe_in = fae_in.cpe_in;
	const auto& viewables = cpe_in.command_in.folder.work->viewables;

	auto& folder = cpe_in.command_in.folder;

	const auto cache = image_cache(image_definition_view(folder.current_path, viewables.image_definitions[new_id]));

	auto& history = folder.history;

	{
		auto cmd = in.command;

		{
			const auto addr = make_field_address(&invariants::sprite::size);
			cmd.property_id = flavour_property_id { in.invariant_id, addr };
		}

		{
			using T = decltype(invariants::sprite::size);
			const auto original_size = T(cache.original_image_size);

			cmd.value_after_change = augs::to_bytes(original_size);
		}

		cmd.built_description = "Set sprite size to image dimensions";
		cmd.common.has_parent = true;

		history.execute_new(std::move(cmd), cpe_in.command_in);
	}

	if (const auto invariant_id = in.shape_polygon_invariant_id) {
		auto cmd = in.command;

		{
			const auto addr = make_field_address(&invariants::shape_polygon::shape);
			cmd.property_id = flavour_property_id { *invariant_id, addr };
		}

		{
			using T = decltype(invariants::shape_polygon::shape);

			const auto original_shape = T(cache.make_box());
			cmd.value_after_change = augs::to_bytes(original_shape);
		}

		cmd.built_description = "Update physics shape to match sprite size";
		cmd.common.has_parent = true;

		history.execute_new(std::move(cmd), cpe_in.command_in);
	}
}
