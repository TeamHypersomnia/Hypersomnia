#include "augs/string/string_templates.h"
#include "game/transcendental/cosmos.h"
#include "application/setups/editor/property_editor/update_size_if_tex_changed.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/detail/find_free_name.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"

void update_size_if_tex_changed(
	const edit_invariant_input in,
	const invariants::sprite&,
	const assets::image_id new_id
) {
	const auto fae_in = in.fae_in;
	const auto cpe_in = fae_in.cpe_in;
	const auto cmd_in = cpe_in.command_in;

	const auto& viewables = cmd_in.get_viewable_defs();
	const auto& image_defs = viewables.image_definitions;

	auto& folder = cmd_in.folder;

	const auto cache = image_cache(image_definition_view(folder.current_path, image_defs[new_id]));

	{
		auto cmd = in.command;

		{
			const auto addr = MACRO_MAKE_FIELD_ADDRESS(invariants::sprite, size);
			cmd.property_id = flavour_property_id { in.invariant_id, addr };
		}

		{
			using T = decltype(invariants::sprite::size);
			const auto original_size = T(cache.original_image_size);

			cmd.value_after_change = augs::to_bytes(original_size);
		}

		cmd.built_description = "Set sprite size to image dimensions";
		cmd.common.has_parent = true;

		post_editor_command(cmd_in, std::move(cmd));
	}

	if (const auto invariant_id = in.shape_polygon_invariant_id) {
		auto cmd = in.command;

		{
			const auto addr = MACRO_MAKE_FIELD_ADDRESS(invariants::shape_polygon, shape);
			cmd.property_id = flavour_property_id { *invariant_id, addr };
		}

		{
			using T = decltype(invariants::shape_polygon::shape);

			const auto original_shape = T(cache.make_box());
			cmd.value_after_change = augs::to_bytes(original_shape);
		}

		cmd.built_description = "Update physics shape to match sprite size";
		cmd.common.has_parent = true;

		post_editor_command(cmd_in, std::move(cmd));
	}
}
