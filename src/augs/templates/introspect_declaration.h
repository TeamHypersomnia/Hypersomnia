#pragma once

namespace augs {
	template <
		class F, 
		class Instance, 
		class... Instances
	>
	void introspect(
		F&& callback,
		Instance& t,
		Instances&... tn
	);

	template <
		class F, 
		class Instance, 
		class... Instances
	>
	void introspect_if_not_leaf(
		F&& callback,
		Instance& t,
		Instances&... tn
	);

	template <class A, class B>
	bool equal_or_introspective_equal(const A& a, const B& b);

	template <class A, class B>
	bool introspective_equal(const A& a, const B& b);
}