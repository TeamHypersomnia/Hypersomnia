#pragma once

template <class Derived, class GenericId>
struct multipool_dispatchers {
	using SpecificTypeId = decltype(GenericId::type_id);

	template <class S, class F>
	static decltype(auto) dispatch_if_impl(S& self, const GenericId& id, F&& callback) {
		return self.on_pool(
			id.type_id,
			[&](auto& pool) -> decltype(auto) {
				return callback(pool.find(id.raw));
			}
		);
	}

	template <class S, class F>
	static void dispatch_on_impl(S& self, const GenericId& id, F&& callback) {
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

	Derived* self() {
		return static_cast<Derived*>(this);
	}

	const Derived* self() const {
		return static_cast<Derived*>(this);
	}

	template <class F>
	decltype(auto) dispatch_if(const GenericId& id, F&& callback) {
		return dispatch_impl(*this, id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) dispatch_if(const GenericId& id, F&& callback) const {
		return dispatch_impl(*this, id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) on_pool(const SpecificTypeId id, F&& callback) {
		return self()->pools.visit(id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) on_pool(const SpecificTypeId id, F&& callback) const {
		return self()->pools.visit(id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each(F&& callback) {
		return self()->pools.for_each_container(std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each(F&& callback) const {
		return self()->pools.for_each_container(std::forward<F>(callback));
	}
};

