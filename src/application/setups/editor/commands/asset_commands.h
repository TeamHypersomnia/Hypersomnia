#pragma once
#include "augs/filesystem/path.h"
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/commands/change_property_command.h"
#include "application/setups/editor/commands/id_allocating_command.h"
#include "application/setups/editor/detail/field_address.h"

#include "view/viewables/all_viewables_defs.h"
#include "view/get_asset_pool.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/maybe_official_path.h"
#include "augs/enums/callback_result.h"

namespace augs {
	struct introspection_access;
}

template <class id_type>
struct create_pathed_asset_id_command : id_allocating_command<id_type> {
	using base = id_allocating_command<id_type>;
	using introspect_base = base;
	using stored_type = typename remove_cref<decltype(get_viewable_pool<id_type>(std::declval<all_viewables_defs&>()))>::mapped_type;

	create_pathed_asset_id_command() = default;
	create_pathed_asset_id_command(stored_type&& from) : construct_from(std::move(from)) {}

	// GEN INTROSPECTOR struct create_pathed_asset_id_command class id_type
	stored_type construct_from;
	// END GEN INTROSPECTOR

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

template <class id_type>
struct create_unpathed_asset_id_command : id_allocating_command<id_type> {
	using base = id_allocating_command<id_type>;
	using introspect_base = base;

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

template <class id_type>
struct duplicate_asset_command : create_unpathed_asset_id_command<id_type> {
	using base = create_unpathed_asset_id_command<id_type>;
	using introspect_base = base;

	// GEN INTROSPECTOR struct duplicate_asset_command class id_type
	id_type duplicate_from;
	// END GEN INTROSPECTOR

	duplicate_asset_command() = default;
	duplicate_asset_command(const id_type duplicate_from) : duplicate_from(duplicate_from) {}

	std::string describe() const;
	void redo(const editor_command_input in);
	using base::undo;
};

template <class id_type>
struct forget_asset_id_command : id_freeing_command<id_type> {
	using base = id_freeing_command<id_type>;
	using base::base;

	using introspect_base = base;

	// GEN INTROSPECTOR struct forget_asset_id_command class id_type
	std::string built_description;
	// END GEN INTROSPECTOR

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

template <class id_type>
struct asset_property_id {
	asset_field_address field;
};

template <class id_type>
struct change_asset_property_command : change_property_command<change_asset_property_command<id_type>> {
	using introspect_base = change_property_command<change_asset_property_command>;

	// GEN INTROSPECTOR struct change_asset_property_command class id_type
	editor_command_common common;

	asset_property_id<id_type> property_id;
	std::vector<id_type> affected_assets;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return affected_assets.size();
	}
};

template <class T>
struct is_create_asset_id_command : std::false_type {};

template <class T>
struct is_create_asset_id_command<create_pathed_asset_id_command<T>> : std::true_type {};

template <class T>
struct is_create_asset_id_command<create_unpathed_asset_id_command<T>> : std::true_type {};

template <class T>
constexpr bool is_create_asset_id_command_v = is_create_asset_id_command<T>::value;

