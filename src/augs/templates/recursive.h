#pragma once

namespace augs {
	template <class Functor>
	struct fix_type {
		Functor functor;

		template <class... Args>
		decltype(auto) operator()(Args&&... args) const& {
			return functor(functor, std::forward<Args>(args)...);
		}
	};

	template <class Functor>
	fix_type<std::decay_t<Functor>> recursive(Functor&& functor) {
		return { std::forward<Functor>(functor) };
	}

	template <class F>
	auto pass_self(F&& callback) {
		return [&](auto&&... args) { callback(std::forward<F>(callback), std::forward<decltype(args)>(args)...); };
	}
}