#pragma once
#include "game/common_state/entity_flavours.h"

#include "application/setups/editor/property_editor/property_editor_gui.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

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

template <class T, class E>
void edit_invariant(
	property_editor_gui& state,
	const T& invariant,
	const entity_flavour<E>& source_flavour,
	const typed_entity_flavour_id<E> flavour_id,
   	const editor_command_input in
) {
	using namespace augs::imgui;

	auto make_property_id = [&](const field_address field) {
		flavour_property_id result;

		result.flavour_id = flavour_id;
		result.invariant_id = index_in_list_v<T, decltype(entity_flavour<E>::invariants)>;
		result.field = field;

		return result;
	};

	const auto property_location = [&]() {
		const auto flavour_name = get_name_of(source_flavour);
		const auto invariant_name = get_invariant_stem(invariant);

		return typesafe_sprintf(" (in %x of %x)", invariant_name, flavour_name);
	}();

	/* Linker error fix */
	auto& history = in.folder.history;

	auto post_new_change = [&](
		const auto& description,
		const flavour_property_id property_id,
		const auto& new_content
	) {
		change_flavour_property_command cmd;
		cmd.property_id = property_id;

		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<change_flavour_property_command>(std::addressof(last))) {
			cmd->built_description = description + property_location;
			cmd->rewrite_change(augs::to_bytes(new_content), in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	general_edit_properties(
		state, 
		invariant,
		make_property_id,
		post_new_change,
		rewrite_last_change
	);
}

template <class E>
void edit_flavour(
	property_editor_gui& state,
	const typed_entity_flavour_id<E> flavour_id,
	const entity_flavour<E>& flavour,
   	const editor_command_input in
) {
	using namespace augs::imgui;

	edit_invariant(state, flavour.template get<invariants::name>(), flavour, flavour_id, in);

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
				edit_invariant(state, invariant, flavour, flavour_id, in);
			}
		}
   	);
}
