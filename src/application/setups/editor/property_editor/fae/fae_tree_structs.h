#pragma once
#include <optional>
#include <unordered_set>

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_flavour_id.h"
#include "application/setups/editor/property_editor/commanding_property_editor_input.h"
#include "game/cosmos/per_entity_type.h"

enum class fae_view_type {
	// GEN INTROSPECTOR enum class fae_view_type
	FLAVOURS,
	ENTITIES
	// END GEN INTROSPECTOR
};

struct entities_tree_state {
	entity_id hovered_id;
};

struct fae_property_editor_input {
	commanding_property_editor_input cpe_in;
};

struct fae_tree_input {
	commanding_property_editor_input cpe_in;
	const bool show_filter_buttons = false;
	const bool show_flavour_control_buttons = false;
	const bool show_locations_using_flavour = false;
	const bool show_instantiation_button = false;

	operator fae_property_editor_input() const {
		return { cpe_in };
	}
};

struct fae_tree_filter {
	std::optional<entity_type_id> deselect_type_id;
	std::optional<entity_flavour_id> deselect_flavour_id;

	std::optional<entity_type_id> select_only_type_id;
	std::optional<entity_flavour_id> select_only_flavour_id;

	void perform(
		const cosmos&,
		std::unordered_set<entity_id>& selections
	) const;

	bool any() const;
};

struct change_flavour_property_command;

struct edit_invariant_input {
	const fae_property_editor_input fae_in;
	const unsigned edited_invariant_id;
	const unsigned text_details_invariant_id;
	const std::optional<unsigned> shape_polygon_invariant_id;
	const std::optional<unsigned> sprite_invariant_id;
	const std::string& source_flavour_name;
	const change_flavour_property_command& command;
};
