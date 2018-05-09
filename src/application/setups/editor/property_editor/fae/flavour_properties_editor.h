#pragma once
#include "augs/string/get_type_name.h"
#include "game/common_state/entity_flavours.h"

#include "application/setups/editor/property_editor/compare_all_fields_to.h"

#include "application/setups/editor/property_editor/general_edit_properties.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

#include "application/setups/editor/property_editor/widgets/pathed_asset_widget.h"
#include "application/setups/editor/property_editor/widgets/flavour_widget.h"
#include "application/setups/editor/property_editor/widgets/unpathed_asset_widget.h"
#include "application/setups/editor/property_editor/update_size_if_tex_changed.h"

#include "application/setups/editor/property_editor/special_widgets.h"
#include "application/setups/editor/detail/format_struct_name.h"

template <class T>
decltype(auto) get_name_of(const entity_flavour<T>& flavour) {
	return flavour.template get<invariants::text_details>().name;
}

template <class C, class T, class N>
void describe_if_rename(C& cmd, std::string& old, const field_address field, const T& invariant, const N& new_content) {
	if constexpr(std::is_same_v<T, invariants::text_details> && std::is_same_v<N, std::string>) {
		if (field == make_field_address(&invariants::text_details::name)) {
			if (old.empty()) {
				old = "Renamed " + invariant.name;
			}

			cmd.built_description = old + " to " + new_content;
		}
	}
	else {
		(void)cmd; (void)old; (void)field; (void)invariant; (void)new_content;
	}
}

template <class T>
void edit_invariant(
	const T& invariant,
	edit_invariant_input in
) {
	using command_type = remove_cref<decltype(in.command)>;
	using namespace augs::imgui;

	const auto fae_in = in.fae_in;

	auto& cpe_in = fae_in.cpe_in;
	auto& cmd_in = cpe_in.command_in;

	const auto property_location = [&]() {
		const auto flavour_name = in.source_flavour_name;
		const auto invariant_name = format_struct_name(invariant);

		return typesafe_sprintf(" (in %x of %x)", invariant_name, flavour_name);
	}();

	/* Linker error fix */
	auto& history = cmd_in.folder.history;

	auto& defs = cmd_in.folder.work->viewables;
	auto& old_description = cpe_in.prop_in.state.old_description;

	auto post_new_change = [&](
		const auto& description,
		const auto field_id,
		const auto& new_content
	) {
		{
			auto cmd = in.command;

			cmd.property_id = flavour_property_id { in.invariant_id, field_id };
			cmd.value_after_change = augs::to_bytes(new_content);
			cmd.built_description = description + property_location;
			describe_if_rename(cmd, old_description, field_id, invariant, new_content);

			history.execute_new(std::move(cmd), cmd_in);
		}

		update_size_if_tex_changed(in, invariant, new_content);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<command_type>(std::addressof(last))) {
			cmd->built_description = description + property_location;
			describe_if_rename(*cmd, old_description, cmd->property_id.field, invariant, new_content);
			cmd->rewrite_change(augs::to_bytes(new_content), cmd_in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	const auto& cosm = cmd_in.get_cosmos();
	
	const auto project_path = cmd_in.folder.current_path;

	general_edit_properties(
		cpe_in.prop_in, 
		invariant,
		post_new_change,
		rewrite_last_change,
		[&](const auto& first, const field_address field_id) {
			return compare_all_fields_to(
				first,
				flavour_property_id { in.invariant_id, field_id }, 
				cosm, 
				in.command.type_id, 
				in.command.affected_flavours
			);
		},
		special_widgets(
			pathed_asset_widget { defs, project_path, cmd_in },
			unpathed_asset_widget { defs, cosm.get_logical_assets() },
			flavour_widget { cosm }
		),
		asset_sane_default_provider { defs }
	);
}


template <class E>
void edit_flavour(
	const fae_property_editor_input in,
	const entity_flavour<E>& flavour,
	const change_flavour_property_command& command
) {
	using namespace augs::imgui;

	auto get_index = [](const auto& inv) {
		return index_in_list_v<remove_cref<decltype(inv)>, invariants_of<E>>;
	};

	const auto& name_invariant = flavour.template get<invariants::text_details>();
	const auto source_flavour_name = name_invariant.name;

	/*
		Show the name invariant as the first one,
		because renaming might be a common operation.
	*/

	std::optional<unsigned> shape_polygon_invariant_id;
	
	if constexpr(entity_flavour<E>::template has<invariants::shape_polygon>()) {
		shape_polygon_invariant_id = static_cast<unsigned>(get_index(invariants::shape_polygon()));
	}

	auto do_edit_invariant = [&](const auto& invariant) {
		edit_invariant(invariant, edit_invariant_input {
			in,
			static_cast<unsigned>(get_index(invariant)),
			shape_polygon_invariant_id,
			source_flavour_name,
			command
		});
	};

	do_edit_invariant(name_invariant);

	for_each_through_std_get(
		flavour.invariants,
		[&do_edit_invariant](auto& invariant) {
			using T = remove_cref<decltype(invariant)>;

			if constexpr(std::is_same_v<T, invariants::text_details>) {
				/* This one is handled already */
				return;
			}

			const auto invariant_label = format_struct_name(invariant) + " invariant";
			const auto node = scoped_tree_node_ex(invariant_label);
			next_column_text();

			if (node) {
				do_edit_invariant(invariant);
			}
		}
   	);
}
