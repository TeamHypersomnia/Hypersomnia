#include "vector_wrapper.h"

namespace augs {
	vector_wrapper<wchar_t> towchar_vec(const std::wstring& s) {
		vector_wrapper<wchar_t> out;

		for (auto& c : s) {
			out.push_back(c);
		}

		return out;
	}
}