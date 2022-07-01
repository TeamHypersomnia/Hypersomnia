#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "augs/ensure_rel.h"
#include "augs/ensure_rel_util.h"
#include "augs/misc/pool/pool_structs.h"

template <class id_type>
struct allocating_command {
	editor_command_meta meta;

private:
	id_type allocated_id;
public:

	auto get_allocated_id() const {
		return allocated_id;
	}

	template <class P, class... Args>
	auto& redo(P& pool, Args&&... args) {
		const auto previous_id = allocated_id;

		auto validate = [previous_id](const auto new_id) {
			if (previous_id.is_set()) {
				(void)new_id;
				ensure_eq_id(new_id, previous_id);
			}
		};

		const auto allocation = pool.allocate(std::forward<Args>(args)...);

		{
			const auto new_id = allocation.key;
			validate(new_id);
			allocated_id = new_id;
		}

		return allocation.object;
	}

	template <class P>
	auto& redo_and_copy(P& pool, const id_type& source) {
		auto& obj = redo(pool);
		obj = pool[source];
		return obj;
	}

	template <class P>
	void undo(P& pool) {
		pool.undo_last_allocate(allocated_id);
	}
};
