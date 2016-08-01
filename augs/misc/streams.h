#pragma once
#include <vector>

namespace augs {
	class output_stream_reserver;

	class stream_position {
	protected:
		size_t pos = 0;
	public:

		size_t get_pos() const {
			return pos;
		}

		void reset_pos() {
			pos = 0;
		}

		size_t size() const {
			return pos;
		}
	};

	class stream : public stream_position {
	public:
		std::vector<char> buf;

		bool operator==(const stream&) const;

		char* data();
		const char* data() const;
		
		void read(char* data, size_t bytes);
		void write(const char* data, size_t bytes);
		void reserve(size_t);
		void reserve(const output_stream_reserver&);
	};

	class bits_stream : public stream_position {
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
	};

	class output_stream_reserver : public stream_position {
	public:
		stream make_stream();

		void write(const char* data, size_t bytes);
	};
}