#pragma once
#include <vector>
#include "ptr_wrapper.h"

namespace augs {
	/* vector wrapper that is used to facilitate binding to lua */
	template<class value>
	struct vector_wrapper {
		std::vector<value> raw;

		void add(value v) {
			raw.push_back(v);
		}

		void push_back(const value& v) {
			raw.push_back(v);
		}

		ptr_wrapper<value> data() {
			return raw.data();
		}

		int size() {
			return raw.size();
		}

		value at(int i) {
			return raw.at(i);
		}
	};

	extern vector_wrapper<wchar_t> towchar_vec(const std::wstring& s);
}