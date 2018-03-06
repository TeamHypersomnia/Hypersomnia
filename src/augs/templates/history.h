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
		bool modified_flag = false;

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

		auto& last_command() {
			return commands[current_revision];
		}

		auto& next_command() {
			return commands[current_revision + 1];
		}

	public:

		template <class T, class... RedoArgs>
		void execute_new(T&& command, RedoArgs&&... redo_args) {
			/* 
				Remove all redoable entries past the current revision.
				Later we might support branches. 
			*/

			modified_flag = true;

			erase_from_to(commands, current_revision + 1);

			if (saved_at_revision 
				&& saved_at_revision.value() >= current_revision + 1
			) {
				/* The revision that has been saved has just been deleted */
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

		void mark_as_not_modified() {
			modified_flag = false;
		}

		void mark_as_just_saved() {
			mark_as_not_modified();
			mark_current_revision_as_saved();
		}

		bool at_unsaved_revision() const {
			return saved_at_revision != current_revision;
		}

		bool was_modified() const {
			return modified_flag;
		}

		template <class... Args>
		void redo(Args&&... args) {
			if (is_revision_newest()) {
				return;
			}

			modified_flag = true;

			std::visit(
				[&](auto& command) {
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

			modified_flag = true;

			std::visit(
				[&](auto& command) {
					command.undo(std::forward<Args>(args)...); 
				},
				last_command()
			);

			--current_revision;
		}

		template <class... Args>
		void seek_to_revision(const index_type n, Args&&... args) {
			while (current_revision < n) {
				redo(std::forward<Args>(args)...);
			}

			while (current_revision > n) {
				undo(std::forward<Args>(args)...);
			}
		}

		const auto& get_commands() const {
			return commands;
		}

		auto get_current_revision() const {
			return current_revision;
		}
	};
}
