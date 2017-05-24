#pragma once
#include <type_traits>

namespace augs {
	template <bool Condition>
	struct constexpr_if {
	
	};
	
	template <>
	struct constexpr_if<true> {
		struct elser {
			template <
				class F,
				class... Args
			> 
			void _else(F&&, Args&&...) {
	
			}
	
			template <
				bool C,
				class F,
				class... Args
			> 
			auto _else_if(F&&, Args&&...) {
				return elser();
			}
		};
	
		template <
			class F,
			class... Args
		>
		auto operator()(F&& callback, Args&&... args) {
			callback(std::forward<Args>(args)...);
			return elser();
		}
	};
	
	template <>
	struct constexpr_if<false> {
		struct elser {
			template <
				bool C,
				class F,
				class... Args
			>
			auto _else_if(F&& callback, Args&&... args) {
				return constexpr_if<C>()(std::forward<F>(callback), std::forward<Args>(args)...);
			}
	
			template <
				class F,
				class... Args
			>
			void _else(F&& callback, Args&&... args) {
				callback(std::forward<Args>(args)...);
			}
		};
	
		template <
			class F,
			class... Args
		>
		auto operator()(F&& callback, Args&&... args) {
			return elser();
		}
	};
}