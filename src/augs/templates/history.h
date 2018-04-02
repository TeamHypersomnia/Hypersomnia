#pragma once
#include <vector>
#include <variant>
#include <optional>

#include "augs/templates/container_templates.h"

namespace augs {
	struct introspection_access;

	template <class Derived, class... CommandTypes>
	class history {
	public:
		using command_type = std::variant<CommandTypes...>;
		using index_type = int;

	private:
		friend augs::introspection_access;

		// GEN INTROSPECTOR class augs::history class Derived class... CommandTypes 
		index_type current_revision = static_cast<index_type>(-1);
		std::vector<command_type> commands; 
		// END GEN INTROSPECTOR

		void derived_set_modified_flags() {
			auto& self = *static_cast<Derived*>(this);
			self.set_modified_flags();
		}

	public:
		template <class T, class... RedoArgs>
		void execute_new(T&& command, RedoArgs&&... redo_args) {
			/* 
				Remove all redoable entries past the current revision.
				Later we might support branches. 
			*/

			erase_from_to(commands, current_revision + 1);

			derived_set_modified_flags();

			{
				auto& self = *static_cast<Derived*>(this);
				self.invalidate_revisions_from(current_revision + 1);
			}

			commands.emplace_back(
				std::in_place_type_t<std::decay_t<T>>(),
				std::forward<T>(command)
			); 

			redo(std::forward<RedoArgs>(redo_args)...);
		}

		template <class... Args>
		bool redo(Args&&... args) {
			if (is_revision_newest()) {
				return false;
			}

			derived_set_modified_flags();

			std::visit(
				[&](auto& command) {
					command.redo(std::forward<Args>(args)...); 
				},
				next_command()
			);

			++current_revision;
			return true;
		}

		template <class... Args>
		bool undo(Args&&... args) {
			if (is_revision_oldest()) {
				return false;
			}

			derived_set_modified_flags();

			std::visit(
				[&](auto& command) {
					command.undo(std::forward<Args>(args)...); 
				},
				last_command()
			);

			--current_revision;
			return true;
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

		bool is_revision_newest() const {
			return current_revision == static_cast<index_type>(commands.size()) - 1;
		}

		bool is_revision_oldest() const {
			return current_revision == static_cast<index_type>(-1);
		}
		
		bool has_last_command() const {
			return !is_revision_oldest();
		}

		auto& last_command() {
			return commands[current_revision];
		}

		auto& next_command() {
			return commands[current_revision + 1];
		}

		const auto& last_command() const {
			return commands[current_revision];
		}

		const auto& next_command() const {
			return commands[current_revision + 1];
		}

		bool empty() const {
			return commands.empty();
		}
	};

	template <class... CommandTypes>
	class history_with_marks : public augs::history<history_with_marks<CommandTypes...>, CommandTypes...> {
		using base = augs::history<history_with_marks<CommandTypes...>, CommandTypes...>;
		friend base;

	public:
		using index_type = typename base::index_type;

	private:
		friend augs::introspection_access;

		// GEN INTROSPECTOR class history_with_marks class... CommandTypes
		// INTROSPECT BASE augs::history<history_with_marks<CommandTypes...>, CommandTypes...>
		std::optional<index_type> saved_at_revision;
		bool modified_since_save = false;
		// END GEN INTROSPECTOR

		void set_modified_flags() {
			modified_since_save = true;
		}

		void invalidate_revisions_from(const index_type index) {
			if (saved_at_revision && saved_at_revision.value() >= index) {
				/* The revision that is currently saved to disk has just been deleted */
				saved_at_revision = std::nullopt;
			}
		}

	public:
		using base::get_current_revision;
		using base::empty;

		void mark_current_revision_as_saved() {
			saved_at_revision = get_current_revision();
		}

		void mark_as_not_modified() {
			modified_since_save = false;
		}

		void mark_as_just_saved() {
			mark_as_not_modified();
			mark_current_revision_as_saved();
		}

		bool is_revision_saved(const index_type candidate) const {
			return saved_at_revision == candidate;
		}

		bool at_unsaved_revision() const {
			return !empty() && !is_revision_saved(get_current_revision());
		}

		bool was_modified() const {
			return modified_since_save;
		}
	};
}
