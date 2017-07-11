#pragma once
#include <type_traits>

namespace augs {
	template <class F>
	class scope_guard {
	public:
		scope_guard(F&& exit_function) : 
			exit_function(std::move(exit_function))
		{}

		scope_guard(scope_guard&& f) :
			exit_function(std::move(f.exit_function)),
			execute_on_destruction(f.execute_on_destruction) 
		{
			f.release();
		}

		~scope_guard() {
			if (execute_on_destruction) {
				exit_function();
			}
		}

		void release() {
			execute_on_destruction = false;
		}

		scope_guard(const scope_guard&) = delete;
		scope_guard& operator=(const scope_guard&) = delete;
		scope_guard& operator=(scope_guard&&) = delete;

	private:
		F exit_function;
		bool execute_on_destruction = true;
	};

	template <class F>
	scope_guard<F> make_scope_guard(F&& exit_function) {
		return scope_guard<F>{std::forward<F>(exit_function)};
	}
}
