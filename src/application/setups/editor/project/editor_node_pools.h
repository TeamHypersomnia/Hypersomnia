#pragma once
#include <unordered_map>
#include "augs/misc/pool/pool.h"
#include "application/setups/editor/nodes/all_editor_node_types.h"
#include "application/setups/editor/nodes/editor_node_id.h"

template <class T>
using make_editor_node_pool = augs::pool<T, make_vector, editor_node_pool_size_type>;
using all_editor_node_pools = per_type_container<all_editor_node_types, make_editor_node_pool>;

struct editor_node_pools {
	static constexpr bool json_ignore = true;

	all_editor_node_pools pools;
	std::unordered_map<editor_node_id, std::string> node_names;

	template <class S, class F>
	static decltype(auto) dispatch_if_impl(S& self, const editor_node_id& id, F&& callback) {
		return self.on_pool(
			id.type_id,
			[&](auto& pool) -> decltype(auto) {
				return callback(pool.find(id.raw));
			}
		);
	}

	template <class S, class F>
	static void dispatch_on_impl(S& self, const editor_node_id& id, F&& callback) {
		return self.on_pool(
			id.type_id,
			[&](auto& pool) {
				auto found = pool.find(id.raw);
				
				if (found) {
					callback(*found);
				}
			}
		);
	}

	template <class F>
	decltype(auto) dispatch_if(const editor_node_id& id, F&& callback) {
		return dispatch_impl(*this, id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) dispatch_if(const editor_node_id& id, F&& callback) const {
		return dispatch_impl(*this, id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) on_pool(const editor_node_type_id id, F&& callback) {
		return pools.visit(id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) on_pool(const editor_node_type_id id, F&& callback) const {
		return pools.visit(id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each_node_pool(F&& callback) {
		return pools.for_each_container(std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each_node_pool(F&& callback) const {
		return pools.for_each_container(std::forward<F>(callback));
	}
};
