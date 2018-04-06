#pragma once
#include <optional>
#include <unordered_set>

#include "application/setups/editor/property_editor/property_editor_structs.h"

struct flavours_and_entities_tree_input {
	property_editor_input prop_in;
	const bool show_filter_buttons = false;
};

struct flavours_and_entities_tree_filter {
	std::optional<entity_type_id> close_type_id;
	std::optional<entity_flavour_id> close_flavour_id;

	std::optional<entity_type_id> only_type_id;
	std::optional<entity_flavour_id> only_flavour_id;

	void perform(
		const cosmos&,
		std::unordered_set<entity_id>& selections
	) const;

	bool any() const;
};

