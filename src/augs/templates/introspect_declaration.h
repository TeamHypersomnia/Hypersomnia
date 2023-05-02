#pragma once

namespace augs {
	template <
		class F, 
		class Instance, 
		class... Instances
	>
	void introspect(
		F callback,
		Instance& t,
		Instances&... tn
	);
}