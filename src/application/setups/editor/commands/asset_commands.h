#pragma once
#include "augs/filesystem/path.h"
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "view/viewables/all_viewables_declarations.h"

template <class id_type>
struct create_asset_id_command {
	// GEN INTROSPECTOR struct create_asset_id_command class id_type
	editor_command_common common;
private:
	id_type allocated_id;
public:
	// END GEN INTROSPECTOR

	auto get_allocated_id() const {
		return allocated_id;
	}

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

template <class id_type>
struct forget_asset_id_command {
	// GEN INTROSPECTOR struct forget_asset_id_command class id_type
	id_type forgotten_id;
	// END GEN INTROSPECTOR

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};
