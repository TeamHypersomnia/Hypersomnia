#pragma once
#include "augs/templates/recursive.h"

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
	bool introspective_equal(const A& a, const B& b);
}