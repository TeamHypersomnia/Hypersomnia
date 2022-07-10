#pragma once
#include "application/setups/debugger/debugger_command_input.h"
#include "augs/ensure_rel.h"
#include "augs/ensure_rel_util.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/misc/pool/pool_structs.h"

template <class id_type>
struct freeing_command {
	debugger_command_common common;

	id_type freed_id;

private:
	friend augs::introspection_access;
	typename id_type::undo_free_type undo_free_input;
	std::vector<std::byte> forgotten_content;
public:

	id_freeing_command() = default;
	id_freeing_command(const id_type freed_id) : freed_id(freed_id) {}

	template <class P>
	void redo(P& pool) {
		clear_undo_state();

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

		clear_undo_state();
	}

	void clear_undo_state() {
		forgotten_content.clear();
	}
};
