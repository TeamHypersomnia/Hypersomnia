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

		char& operator[](size_t);
		const char& operator[](size_t) const;

		std::string to_string() const;
		
		void read(char* data, size_t bytes);
		void write(const char* data, size_t bytes);
		void reserve(size_t);
		void reserve(const output_stream_reserver&);
	};

	class output_stream_reserver : public stream_position {
	public:
		stream make_stream();

		void write(const char* data, size_t bytes);
	};
}