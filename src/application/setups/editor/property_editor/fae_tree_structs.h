#pragma once
#include <optional>
#include <unordered_set>

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_flavour_id.h"
#include "application/setups/editor/property_editor/commanding_property_editor_input.h"
#include "game/transcendental/entity_type_templates.h"

enum class fae_view_mode {
	// GEN INTROSPECTOR enum class fae_view_mode
	FLAVOURS,
	ENTITIES
	// END GEN INTROSPECTOR
};

struct fae_tree_state {
	entity_guid hovered_guid;
	fae_view_mode view_mode = fae_view_mode::FLAVOURS;

	template <class T>
	using make_selected_flavours = std::unordered_set<typed_entity_flavour_id<T>>;

	per_entity_type_container<make_selected_flavours> selected_flavours;
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

struct change_flavour_property_command;

struct edit_invariant_input {
	const fae_property_editor_input fae_in;
	const unsigned invariant_id;
	const std::optional<unsigned> shape_polygon_invariant_id;
	const std::string& source_flavour_name;
	const change_flavour_property_command& command;
};
