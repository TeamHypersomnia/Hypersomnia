#pragma once
#include "application/setups/editor/editor_command_input.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/misc/pool/pool_structs.h"

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

template <class id_type>
struct id_freeing_command {
	// GEN INTROSPECTOR struct id_freeing_command class id_type
	editor_command_common common;

	id_type freed_id;

private:
	friend augs::introspection_access;
	typename id_type::undo_free_type undo_free_input;
	std::vector<std::byte> forgotten_content;
public:
	// END GEN INTROSPECTOR

	id_freeing_command() = default;
	id_freeing_command(const id_type freed_id) : freed_id(freed_id) {}

	template <class P>
	void redo(P& pool) {
		ensure(forgotten_content.empty());

		auto s = augs::ref_memory_stream(forgotten_content);
		augs::write_bytes(s, pool.get(freed_id));
		undo_free_input = *pool.free(freed_id);
	}

	template <class P>
	void undo(P& pool) {
		auto s = augs::cref_memory_stream(forgotten_content);

		typename P::mapped_type def;
		augs::read_bytes(s, def);
		pool.undo_free(undo_free_input, std::move(def));

		forgotten_content.clear();
	}
};
