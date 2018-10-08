#pragma once
#include <vector>
#include <variant>
#include <optional>
#include "augs/misc/time_utils.h"

namespace augs {
	struct introspection_access;

	enum class history_op_type {
		NONE,

		UNDO,
		REDO,
		EXECUTE_NEW,
		SEEK,
		FORCE_SET_REVISION
	};

	struct last_history_op {
		// GEN INTROSPECTOR struct last_history_op
		history_op_type type = history_op_type::NONE;
		date_time stamp;
		// END GEN INTROSPECTOR
	};

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
		last_history_op last_op;
		// END GEN INTROSPECTOR

		void set_last_op(const history_op_type type) {
			last_op = { type, date_time() };
		}

		void derived_set_modified_flags() {
			auto& self = *static_cast<Derived*>(this);
			self.set_modified_flags();
		}

	public:
		const auto& get_last_op() const {
			return last_op;
		}

		template <class T, class... RedoArgs>
		const T& execute_new(T&& command, RedoArgs&&... redo_args);

		template <class... Args>
		bool redo(Args&&... args);

		template <class... Args>
		bool undo(Args&&... args);

		template <class... Args>
		void seek_to_revision(const index_type n, Args&&... args);

		const auto& get_commands() const {
			return commands;
		}

		auto get_current_revision() const {
			return current_revision;
		}

		void force_set_current_revision(const index_type& new_revision) {
			current_revision = new_revision;
			set_last_op(history_op_type::FORCE_SET_REVISION);
		}

		void discard_later_revisions();

		static auto get_first_revision() {
			return static_cast<index_type>(-1);
		}

		auto get_last_revision() const {
			return static_cast<index_type>(commands.size()) - 1;
		}

		bool is_revision_newest() const {
			return current_revision == get_last_revision();
		}

		bool is_revision_oldest() const {
			return current_revision == static_cast<index_type>(-1);
		}
		
		bool has_last_command() const {
			return !is_revision_oldest();
		}

		bool has_next_command() const {
			return !is_revision_newest();
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
		using introspect_base = base;

	private:
		friend augs::introspection_access;

		// GEN INTROSPECTOR class history_with_marks class... CommandTypes
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
