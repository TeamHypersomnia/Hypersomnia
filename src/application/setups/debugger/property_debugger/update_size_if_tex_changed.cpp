#include "augs/string/string_templates.h"
#include "game/cosmos/cosmos.h"
#include "application/setups/debugger/property_debugger/update_size_if_tex_changed.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/detail/find_free_name.h"
#include "application/setups/debugger/debugger_history.hpp"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/to_bytes.h"

void update_size_for_new_image(
	const edit_invariant_input in,
	const assets::image_id new_id
) {
	const auto fae_in = in.fae_in;
	const auto cpe_in = fae_in.cpe_in;
	const auto cmd_in = cpe_in.command_in;

	const auto& viewables = cmd_in.get_viewable_defs();
	const auto& image_defs = viewables.image_definitions;

	const auto& folder = cmd_in.folder;
	ensure(in.sprite_invariant_id.has_value());
	const auto invariant_id = in.sprite_invariant_id.value();

	const auto cache = image_cache(image_definition_view(folder.current_path, image_defs[new_id]));

	{
		auto cmd = in.command;

		{
			const auto addr = MACRO_MAKE_FLAVOUR_FIELD_ADDRESS(invariants::sprite, size);
			cmd.property_id = flavour_property_id { invariant_id, addr };
		}

		{
			using T = decltype(invariants::sprite::size);
			const auto original_size = T(cache.original_image_size);

			augs::assign_bytes(cmd.value_after_change, original_size);
			cmd.built_description = typesafe_sprintf("Set sprite size to image dimensions: %x", original_size);
		}

		cmd.common.has_parent = true;

		post_debugger_command(cmd_in, std::move(cmd));
	}

	const auto& cosm = cmd_in.get_cosmos();

	const auto& name_from_image = ::get_displayed_name(image_defs[new_id]);

	auto make_name_from_image = [](const image_definition& def) {
		return ::cut_trailing_number_and_spaces(::get_displayed_name(def));
	};

	for (const auto& raw_flavour_id : in.command.affected_flavours) {
		const auto flavour_id = entity_flavour_id { raw_flavour_id, in.command.type_id };

		cosm.on_flavour(flavour_id, [&](const auto& flavour) {
			const bool is_duplicate_or_created = [&]() {
				const auto name = cut_trailing_number(flavour.get_name());

				static const std::string dup_suff = "-Dup-";
				static const std::string new_suff = "-New-";

				return 
					ends_with(name, dup_suff)
					|| ends_with(name, new_suff)
				;
			}();

			const bool begins_with_some_image_name = [&]() {
				const auto name = flavour.get_name();

				for (const auto& def : image_defs) {
					if (::begins_with(name, make_name_from_image(def))) {
						return true;
					}
				}

				return false;
			}();

			if (is_duplicate_or_created || begins_with_some_image_name) {
				const auto new_name = ::find_free_name(
					cosm.get_common_significant().flavours.get_for<entity_type_of<decltype(flavour)>>(),
					name_from_image,
					"-"
				);

				auto cmd = in.command;
				cmd.affected_flavours = { raw_flavour_id };
				cmd.property_id.invariant_id = in.text_details_invariant_id;
				cmd.property_id.field = MACRO_MAKE_FLAVOUR_FIELD_ADDRESS(invariants::text_details, name);
				cmd.common.has_parent = true;
				cmd.built_description = "Updated flavour name to name of the chosen image";
				augs::assign_bytes(cmd.value_after_change, new_name);

				post_debugger_command(cmd_in, std::move(cmd));
			}
		});
	}
}

void update_size_if_tex_changed(
	const edit_invariant_input in,
	const flavour_field_address& address,
	const invariants::animation& animation_def
) {
	if (address != MACRO_MAKE_FLAVOUR_FIELD_ADDRESS(invariants::animation, id)) {
		return;
	}

	const auto new_id = animation_def.id;

	const auto fae_in = in.fae_in;
	const auto cpe_in = fae_in.cpe_in;
	const auto cmd_in = cpe_in.command_in;

	const auto& cosm = cmd_in.get_cosmos();
	const auto anim = cosm.get_logical_assets().find(new_id);

	if (anim == nullptr) {
		return;
	}

	ensure(in.sprite_invariant_id.has_value());

	const auto first_image = anim->frames[0].image_id;

	{
		auto cmd = in.command;

		{
			const auto addr = MACRO_MAKE_FLAVOUR_FIELD_ADDRESS(invariants::sprite, image_id);
			cmd.property_id = flavour_property_id { *in.sprite_invariant_id, addr };
		}

		{
			augs::assign_bytes(cmd.value_after_change, first_image);
			cmd.built_description = typesafe_sprintf("Set sprite image id first frame of animation");
		}

		cmd.common.has_parent = true;

		post_debugger_command(cmd_in, std::move(cmd));
	}

	update_size_for_new_image(in, first_image);
}

void update_size_if_tex_changed(
	const edit_invariant_input in,
	const flavour_field_address& address,
	const invariants::sprite& spr
) {
	const auto new_id = spr.image_id;

	if (address == MACRO_MAKE_FLAVOUR_FIELD_ADDRESS(invariants::sprite, image_id)) {
		update_size_for_new_image(in, new_id);
	}
}
