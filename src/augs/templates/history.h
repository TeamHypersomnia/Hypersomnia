#pragma once
#include <vector>
#include <variant>
#include <optional>

#include "augs/templates/container_templates.h"

namespace augs {
	struct introspection_access;

	template <class... CommandTypes>
	class history {
	public:
		using command_type = std::variant<CommandTypes...>;
		using index_type = int;

	private:
		friend augs::introspection_access;

		// GEN INTROSPECTOR class augs::history class... CommandTypes 
		index_type current_revision = static_cast<index_type>(-1);
		std::optional<index_type> saved_at_revision = static_cast<index_type>(-1);

		std::vector<command_type> commands; 
		// END GEN INTROSPECTOR

		bool is_revision_newest() const {
			return current_revision == static_cast<index_type>(commands.size()) - 1;
		}

		bool is_revision_oldest() const {
			return current_revision == static_cast<index_type>(-1);
		}

		const auto& last_command() const {
			return commands[current_revision];
		}

		const auto& next_command() const {
			return commands[current_revision + 1];
		}

	public:
		template <class T, class... RedoArgs>
		void execute_new(T&& command, RedoArgs&&... redo_args) {
			/* 
				Remove all redoable entries.
				Later we might support branches. 
			*/

			erase_from_to(commands, current_revision + 1);

			if (saved_at_revision 
				&& saved_at_revision.value() >= current_revision + 1
			) {
				/* Revision last saved at has beed deleted */
				saved_at_revision = std::nullopt;
			}

			commands.emplace_back(
				std::in_place_type_t<std::decay_t<T>>(),
				std::forward<T>(command)
			); 

			redo(std::forward<RedoArgs>(redo_args)...);
		}

		void mark_current_revision_as_saved() {
			saved_at_revision = current_revision;
		}

		bool has_unsaved_changes() const {
			return saved_at_revision != current_revision;
		}

		template <class... Args>
		void redo(Args&&... args) {
			if (is_revision_newest()) {
				return;
			}

			std::visit(
				[&](const auto& command) {
					command.redo(std::forward<Args>(args)...); 
				},
				next_command()
			);

			++current_revision;
		}

		template <class... Args>
		void undo(Args&&... args) {
			if (is_revision_oldest()) {
				return;
			}

			std::visit(
				[&](const auto& command) {
					command.undo(std::forward<Args>(args)...); 
				},
				last_command()
			);

			--current_revision;
		}

	};
}
