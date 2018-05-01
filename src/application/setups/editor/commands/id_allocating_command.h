#pragma once
#include "application/setups/editor/editor_command_input.h"

namespace augs {
	struct introspection_access;
}

template <class id_type>
struct id_allocating_command {
	friend augs::introspection_access;
	// GEN INTROSPECTOR struct id_allocating_command class id_type
	editor_command_common common;
private:
	id_type allocated_id;
public:
	// END GEN INTROSPECTOR

	auto get_allocated_id() const {
		return allocated_id;
	}

	template <class P>
	auto& redo(P& pool) {
		const auto previous_id = allocated_id;

		auto validate = [previous_id](const auto new_id) {
			if (previous_id.is_set()) {
				ensure_eq(new_id, previous_id);
			}
		};

		const auto allocation = pool.allocate();

		{
			const auto new_id = allocation.key;
			validate(new_id);
			allocated_id = new_id;
		}

		return allocation.object;
	}

	template <class P>
	void undo(P& pool) {
		pool.undo_last_allocate(allocated_id);
	}
};
