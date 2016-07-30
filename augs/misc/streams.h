#pragma once
#include <vector>

namespace augs {
	class output_stream_reserver;

	class stream {
		size_t pos = 0;
	public:
		std::vector<char> buf;

		bool operator==(const stream&) const;

		size_t size() const;
		char* data();
		const char* data() const;
		
		void reset_pos();
		void read(char* data, size_t bytes);
		void write(const char* data, size_t bytes);
		void reserve(size_t);
		void reserve(const output_stream_reserver&);
	};

	class output_stream_reserver {
	public:
		stream make_stream();

		size_t size = 0;
		void write(const char* data, size_t bytes);
	};

}