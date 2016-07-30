#pragma once
#include <vector>

namespace augs {
	class input_stream {
		size_t pos = 0;
	public:
		std::vector<char> buf;

		void read(char* data, size_t bytes);
	};

	class output_stream {
		size_t pos = 0;
	public:
		std::vector<char> buf;

		bool operator==(const output_stream&) const;

		void write(const char* data, size_t bytes);
		void reserve(size_t);
	};

	class output_stream_unsafe {
		size_t pos = 0;
	public:
		std::vector<char> buf;

		bool operator==(const output_stream_unsafe&) const;

		void write(const char* data, size_t bytes);
		void reserve(size_t);
	};

	class output_stream_reserver {
	public:
		output_stream_unsafe make_stream();

		size_t size = 0;
		void write(const char* data, size_t bytes);
	};

}