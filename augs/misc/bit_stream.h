#pragma once
#include <bitset>
#include "streams.h"

namespace augs {
	typedef stream stream;
	/*
	class stream : public stream_position {
	public:
		std::vector<bool> buf;

		void read_variable_length_index(unsigned short&);
		void write_variable_length_index(unsigned short);

		template<int I>
		void read(std::bitset<I>& bs, const unsigned n) {
			for (size_t i = 0; i < n; ++i)
				bs.set(buf[pos + i]);

			pos += n;
		}

		template<int I>
		void write(const std::bitset<I>& bs, const unsigned n) {
			for (size_t i = 0; i < n; ++i)
				buf.push_back(bs.test(i));

			pos += n;
		}

		template<int I>
		void read(std::bitset<I>& bs) {
			for (size_t i = 0; i < I; ++i)
				bs.set(buf[pos + i]);

			pos += I;
		}

		template<int I>
		void write(const std::bitset<I>& bs) {
			for (size_t i = 0; i < I; ++i)
				buf.push_back(bs.test(i));

			pos += I;
		}
	};*/
}