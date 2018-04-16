#pragma once
#include "augs/string/get_type_name.h"
#include "game/common_state/entity_flavours.h"

#include "application/setups/editor/property_editor/compare_all_fields_to.h"

#include "application/setups/editor/property_editor/general_edit_properties.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

#include "application/setups/editor/property_editor/asset_control_provider.h"
#include "application/setups/editor/property_editor/invariant_field_eq_predicate.h"
#include "application/setups/editor/property_editor/update_size_if_tex_changed.h"

template <class T>
decltype(auto) get_name_of(const entity_flavour<T>& flavour) {
	return flavour.template get<invariants::name>().name;
}

template <class T>
auto get_invariant_stem(const T&) {
	auto result = format_field_name(get_type_name_strip_namespace<T>());
	result[0] = std::toupper(result[0]);

	/* These look ugly with automated names */

	if constexpr(std::is_same_v<T, invariants::sprite>) {
		result = "Sprite";
	}	

	if constexpr(std::is_same_v<T, invariants::polygon>) {
		result = "Polygon";
	}

	return result;
}

template <class T>
void edit_invariant(
	const fae_property_editor_input in,
	const T& invariant,
	const unsigned invariant_id,
	const std::string& source_flavour_name,
	const change_flavour_property_command& command
) {
	using command_type = std::decay_t<decltype(command)>;
	using namespace augs::imgui;

	auto& cpe_in = in.cpe_in;
	auto& cmd_in = cpe_in.command_in;

	const auto property_location = [&]() {
		const auto flavour_name = source_flavour_name;
		const auto invariant_name = get_invariant_stem(invariant);

		return typesafe_sprintf(" (in %x of %x)", invariant_name, flavour_name);
	}();

	/* Linker error fix */
	auto& history = cmd_in.folder.history;

	auto& defs = cmd_in.folder.work->viewables;

	auto post_new_change = [&](
		const auto& description,
		const auto field_id,
		const auto& new_content
	) {
		{
			auto cmd = command;

			cmd.property_id = flavour_property_id { invariant_id, field_id };
			cmd.value_after_change = augs::to_bytes(new_content);
			cmd.built_description = description + property_location;

			history.execute_new(std::move(cmd), cmd_in);
		}

		update_size_if_tex_changed(invariant, in, invariant_id, command, new_content);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<command_type>(std::addressof(last))) {
			cmd->built_description = description + property_location;
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
		invariant_field_eq_predicate { 
			cosm, invariant_id, command.type_id, command.affected_flavours 
		},
		asset_control_provider { defs, project_path, cmd_in }
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
		return index_in_list_v<std::decay_t<decltype(inv)>, invariants_of<E>>;
	};

	const auto& name_invariant = flavour.template get<invariants::name>();
	const auto source_flavour_name = name_invariant.name;

	/*
		Show the name invariant as the first one,
		because renaming might be a common operation.
	*/

	edit_invariant(
		in,
	   	name_invariant,
	   	get_index(name_invariant),
	   	source_flavour_name,
	   	command
	);

	for_each_through_std_get(
		flavour.invariants,
		[&](auto& invariant) {
			using T = std::decay_t<decltype(invariant)>;

			if constexpr(std::is_same_v<T, invariants::name>) {
				/* This one is handled already */
				return;
			}

			const auto invariant_label = get_invariant_stem(invariant) + " invariant";
			const auto node = scoped_tree_node_ex(invariant_label);
			next_column_text();

			if (node) {
				edit_invariant(in, invariant, get_index(invariant), source_flavour_name, command);
			}
		}
   	);
}
