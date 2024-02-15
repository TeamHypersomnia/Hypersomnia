#pragma once
#include <vector>
#include <variant>
#include <optional>
#include "augs/misc/date_time.h"

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

		void derived_set_modified_flag() {
			auto& self = *static_cast<Derived*>(this);
			self.set_dirty_flag();
		}

	public:
		const auto& get_last_op() const {
			return last_op;
		}

		bool executed_new() const {
			return get_last_op().type == history_op_type::EXECUTE_NEW;
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

		auto& get_commands() {
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

		bool on_first_revision() const {
			return get_current_revision() == get_first_revision();
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
	class history_with_saved_revision : public augs::history<history_with_saved_revision<CommandTypes...>, CommandTypes...> {
		using base = augs::history<history_with_saved_revision<CommandTypes...>, CommandTypes...>;
		friend base;

	public:
		using index_type = typename base::index_type;
		using introspect_base = base;

	private:
		friend augs::introspection_access;

		// GEN INTROSPECTOR class augs::history_with_saved_revision class... CommandTypes
		std::optional<index_type> saved_at_revision;
		bool dirty_flag = false;
		// END GEN INTROSPECTOR

		void invalidate_revisions_from(const index_type index) {
			if (saved_at_revision && saved_at_revision.value() >= index) {
				/* The revision that is currently saved to disk has just been deleted */
				saved_at_revision = std::nullopt;
			}
		}

		using base::empty;

	public:
		using base::get_current_revision;

		void set_dirty_flag() {
			dirty_flag = true;
		}

		void mark_as_just_saved() {
			dirty_flag = false;
			saved_at_revision = get_current_revision();
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

		bool is_dirty() const {
			return dirty_flag;
		}

		bool empty() const {
			return base::empty() && !dirty_flag;
		}
	};
}
