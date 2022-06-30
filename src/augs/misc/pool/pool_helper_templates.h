#pragma once

template <class Derived, class GenericId, template <class E> class TypedId>
struct multipool_dispatchers {
	using OnlyTypeId = decltype(GenericId::type_id);

	template <class S, class I>
	static decltype(auto) find_typed_impl(S& self, const TypedId<I>& id) {
		return self.template get_pool_for<I>().find(id.raw);
	}

	template <class S, class F>
	static decltype(auto) dispatch_on_impl(S& self, GenericId id, F&& callback) {
		auto invocation = [&]<typename P>(P& pool) -> decltype(auto) {
			auto found = pool.find(id.raw);
			
			using I = typename P::value_type;
			using R = decltype(callback(*found, TypedId<I> { id.raw }));

			if constexpr(std::is_same_v<void, R>) {
				if (found) {
					callback(*found, TypedId<I> { id.raw });
					return true;
				}

				return false;
			}
			else {
				if (found) {
					return std::optional<R>(callback(*found, TypedId<I> { id.raw }));
				}

				return std::optional<R>();
			}
		};

		if (!id.type_id.is_set()) {
			return decltype(invocation(self.self()->pools.template get_nth<0>()))();
		}

		return self.on_pool(id.type_id, invocation);
	}

	Derived* self() {
		return static_cast<Derived*>(this);
	}

	const Derived* self() const {
		return static_cast<const Derived*>(this);
	}

	template <class F>
	decltype(auto) dispatch_on(const GenericId& id, F&& callback) {
		return dispatch_on_impl(*this, id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) dispatch_on(const GenericId& id, F&& callback) const {
		return dispatch_on_impl(*this, id, std::forward<F>(callback));
	}

	template <class I>
	decltype(auto) find_typed(const TypedId<I>& id) {
		return find_typed_impl(*this, id);
	}

	template <class I>
	decltype(auto) find_typed(const TypedId<I>& id) const {
		return find_typed_impl(*this, id);
	}

	template <class T>
	decltype(auto) get_pool_for() {
		return self()->pools.template get_for<T>();
	}

	template <class T>
	decltype(auto) get_pool_for() const {
		return self()->pools.template get_for<T>();
	}

	template <class F>
	decltype(auto) on_pool(const OnlyTypeId id, F&& callback) {
		return self()->pools.visit(id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) on_pool(const OnlyTypeId id, F&& callback) const {
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

