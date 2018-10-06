#pragma once

struct default_solver_callback {
	template <class... Args>
	void operator()(Args&&...) {}
};

template <
	class PreSolve,
	class PostSolve,
	class PostCleanup
>
struct solver_callbacks_t {
	PreSolve pre_solve;
	PostSolve post_solve;
	PostCleanup post_cleanup;
	
	solver_callbacks_t(
		PreSolve&& pre_solve,
		PostSolve&& post_solve,
		PostCleanup&& post_cleanup
	) :
		pre_solve(std::move(pre_solve)),
		post_solve(std::move(post_solve)),
		post_cleanup(std::move(post_cleanup))
	{}

	template <
		class A = default_solver_callback,
		class B = default_solver_callback,
		class C = default_solver_callback
	>
	auto combine(
		A&& new_pre_solve = default_solver_callback(),
		B&& new_post_solve = default_solver_callback(),
		C&& new_post_cleanup = default_solver_callback()
	) const {
		auto combiner = [=](auto new_one, auto old_one) {
			if constexpr(std::is_same_v<decltype(new_one), default_solver_callback>) {
				return old_one;
			}
			else {
				return [=](auto&&... args) {
					new_one(old_one, std::forward<decltype(args)>(args)...);
				};
			}
		};

		return solver_callbacks_t(
			combiner(new_pre_solve, pre_solve),
			combiner(new_post_solve, post_solve),
			combiner(new_post_cleanup, post_cleanup)
		);
	}
};

template <
	class PreSolve = default_solver_callback,
	class PostSolve = default_solver_callback,
	class PostCleanup = default_solver_callback
>
auto solver_callbacks(
	PreSolve&& pre_solve = default_solver_callback(),
	PostSolve&& post_solve = default_solver_callback(),
	PostCleanup&& post_cleanup = default_solver_callback()
) {
	return solver_callbacks_t(
		std::forward<PreSolve>(pre_solve),
		std::forward<PostSolve>(post_solve),
		std::forward<PostCleanup>(post_cleanup)
	);
}
