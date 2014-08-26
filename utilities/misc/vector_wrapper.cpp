#include "stdafx.h"
#include "vector_wrapper.h"

namespace augs {
	namespace misc {
		vector_wrapper<wchar_t> towchar_vec(const std::wstring& s) {
			misc::vector_wrapper<wchar_t> out;

			for (auto& c : s) {
				out.push_back(c);
			}

			return out;
		}
	}
}