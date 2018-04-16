#pragma once
#include "game/assets/ids/asset_ids.h"

#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/property_editor/fae_tree_structs.h"
#include "application/setups/editor/commands/change_flavour_property_command.h"
#include "application/setups/editor/property_editor/component_field_eq_predicate.h"

template <class T, class C>
void update_size_if_tex_changed(
	const T&, const fae_property_editor_input, unsigned, const change_flavour_property_command&, const C&
) {}

inline void update_size_if_tex_changed(
	const invariants::sprite& invariant,
	const fae_property_editor_input in,
	const unsigned invariant_id,
	const change_flavour_property_command& command,
	const assets::image_id& new_content
) {
	auto cmd = command;

	{
		const auto& size = invariant.size;
		cmd.property_id = flavour_property_id { invariant_id, make_field_address(size, invariant) };
	}

	{
		const auto id = new_content;

		vec2u original_size;

		if (const auto cache = mapped_or_nullptr(in.image_caches, id)) {
			original_size = cache->original_image_size;
		}
		else {
			original_size = in.cpe_in.command_in.folder.work->viewables.image_loadables[id].read_source_image_size();
		}

		const auto original_size_casted = decltype(invariants::sprite::size)(original_size);

		cmd.value_after_change = augs::to_bytes(original_size_casted);
	}

	cmd.built_description = "Set sprite size to image dimensions";

	auto& history = in.cpe_in.command_in.folder.history;

	cmd.common.has_parent = true;

	history.execute_new(std::move(cmd), in.cpe_in.command_in);
}
