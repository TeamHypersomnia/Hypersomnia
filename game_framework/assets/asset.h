#pragma once

namespace assets {
	template<class T, class A>
	class asset {
	public:
		void set(T id) { this->id = id; }

		asset(T id) : id(id) {}
		operator T() { return this->id; }
		T id;

		A* operator->() const {
			return resource_manager.find<A>(id);
		}
	};
}
