#pragma once

namespace templates_detail {
	template<class _Ty,
		_Ty _Val>
		struct integral_constant
	{	// convenient template for integral constant types
		static constexpr _Ty value = _Val;

		typedef _Ty value_type;
		typedef integral_constant<_Ty, _Val> type;

		constexpr operator value_type() const
		{	// return stored value
			return (value);
		}

		constexpr value_type operator()() const
		{	// return stored value
			return (value);
		}
	};

	typedef integral_constant<bool, true> true_type;
	typedef integral_constant<bool, false> false_type;
}
