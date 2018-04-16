#pragma once
#include <optional>
#include <unordered_set>

#include "application/setups/editor/property_editor/commanding_property_editor_input.h"

struct fae_tree_state {
	entity_guid hovered_guid;
};

class loaded_image_caches_map;

struct fae_property_editor_input {
	const loaded_image_caches_map& image_caches;
	commanding_property_editor_input cpe_in;
};

struct fae_tree_input {
	fae_tree_state& state;
	commanding_property_editor_input cpe_in;
	const bool show_filter_buttons = false;
	const loaded_image_caches_map& image_caches;

	operator fae_property_editor_input() const {
		return { image_caches, cpe_in };
	}
};

struct fae_tree_filter {
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

