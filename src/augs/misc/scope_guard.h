#pragma once
#include <type_traits>
#include <optional>
#include <utility>

namespace augs {
	template <class F>
	class scope_guard {
	public:
		scope_guard(F&& exit_function) : 
			exit_function(std::move(exit_function))
		{}

		scope_guard(scope_guard&& f) :
			exit_function(std::move(f.exit_function))
		{
			f.release();
		}

		~scope_guard() {
			finish_scope();
		}

		void release() {
			exit_function.reset();
		}

		explicit operator bool() const {
			return exit_function.has_value();
		}

		scope_guard(const scope_guard&) = delete;
		scope_guard& operator=(const scope_guard&) = delete;
		scope_guard& operator=(scope_guard&&) = delete;

	private:
		void finish_scope() {
			if (exit_function) {
				(*exit_function)();
				release();
			}
		}

		std::optional<F> exit_function;
	};
}
