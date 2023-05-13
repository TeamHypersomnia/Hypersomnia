#pragma once
#include <vector>
#include <variant>

#include "augs/templates/history.h"

#include "application/setups/editor/commands/editor_command_meta.h"

#include "application/setups/editor/commands/create_resource_command.h"

#include "application/setups/editor/commands/edit_resource_command.h"
#include "application/setups/editor/commands/edit_node_command.h"
#include "application/setups/editor/commands/create_layer_command.h"
#include "application/setups/editor/commands/create_node_command.h"

#include "application/setups/editor/commands/reorder_nodes_command.h"
#include "application/setups/editor/commands/reorder_layers_command.h"

#include "application/setups/editor/commands/node_transform_commands.h"
#include "application/setups/editor/commands/delete_nodes_command.h"
#include "application/setups/editor/commands/delete_resources_command.h"
#include "application/setups/editor/commands/clone_nodes_command.h"
#include "application/setups/editor/commands/toggle_active_commands.h"
#include "application/setups/editor/commands/edit_layer_command.h"

#include "application/setups/editor/commands/inspect_command.h"
#include "application/setups/editor/commands/unpack_prefab_command.h"

#include "application/setups/editor/commands/edit_project_settings_command.h"
#include "application/setups/editor/commands/replace_whole_project_command.h"

#include "application/setups/editor/editor_history_declaration.h"

struct editor_history : public editor_history_base {
	using base = editor_history_base;
	using base::base;

	void invalidate_revisions_from(const index_type index) {
		if (saved_at_revision && saved_at_revision.value() >= index) {
			/* The revision that is currently saved to disk has just been deleted */
			saved_at_revision = std::nullopt;
		}

		if (last_autosaved_revision && last_autosaved_revision.value() >= index) {
			last_autosaved_revision = std::nullopt;
		}
	}

	void set_dirty_flag() {}

	void mark_revision_as_saved() {
		saved_at_revision = get_current_revision();
		last_autosaved_revision = std::nullopt;
	}

	void mark_revision_as_autosaved() {
		last_autosaved_revision = get_current_revision();
	}

	auto find_saved_revision() const {
		return saved_at_revision;
	}

	bool is_saved_revision(const index_type candidate) const {
		return saved_at_revision == candidate;
	}

	bool at_saved_revision() const {
		return is_saved_revision(get_current_revision()) || empty();
	}

	bool at_unsaved_revision() const {
		return !at_saved_revision();
	}

	bool at_autosaved_revision() const {
		return last_autosaved_revision == get_current_revision();
	}

private:
	std::optional<index_type> last_autosaved_revision;
	std::optional<index_type> saved_at_revision;
};
